/**************************************************************************/
/*  gdmatplot.cpp                                                         */
/**************************************************************************/
/* Copyright (c) 2024 Okan Demir                                          */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "gdmatplot.h"

#include <cstdlib>
#include <mutex>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/core/mutex_lock.hpp>

#include "gdmatplot_gnuplot_library.h"

#if defined(LINUX_ENABLED)
#define GDMATPLOT_DEFAULT_LIBGNUPLOT_PATH "user://libgnuplot.so"
#elif defined(WINDOWS_ENABLED)
#define GDMATPLOT_DEFAULT_LIBGNUPLOT_PATH "user://libgnuplot.dll"
#endif
#define GDMATPLOT_LIB_READ_SIZE 2048

using namespace godot;

static std::mutex _load_library_lock;

void GDMatPlotNative::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_gnuplot", "p_path"), &GDMatPlotNative::load_gnuplot,
						 DEFVAL(String(GDMATPLOT_DEFAULT_LIBGNUPLOT_PATH)));
	ClassDB::bind_method(D_METHOD("run_command", "p_cmd"), &GDMatPlotNative::run_command);
	ClassDB::bind_method(D_METHOD("set_dataframe", "p_data", "p_column_count"),
						 &GDMatPlotNative::set_dataframe);
	ClassDB::bind_method(D_METHOD("load_dataframe"), &GDMatPlotNative::load_dataframe);
	ClassDB::bind_method(D_METHOD("get_version"), &GDMatPlotNative::get_version);
	ClassDB::bind_method(D_METHOD("get_gnuplot_version"), &GDMatPlotNative::get_gnuplot_version);
}

GDMatPlotNative::GDMatPlotNative() {
	_library.instantiate();
}

Variant GDMatPlotNative::load_gnuplot(String p_path) {
	int64_t tmp_rand = std::rand();
	String tmp_path = p_path + "." + String::num_int64(tmp_rand);
	while (FileAccess::file_exists(tmp_path)) {
		tmp_rand = std::rand();
		tmp_path = p_path + "." + String::num_int64(tmp_rand);
	}

	Ref<FileAccess> fh = FileAccess::open(tmp_path, FileAccess::WRITE);
	if (fh == nullptr) {
		return ERR_LIBGNUPLOT_NOT_WRITTEN;
	}

	PackedByteArray tmp_lib;

	uint32_t lib_size = 0;
	const uint8_t *lib_data = nullptr;

	{
		std::lock_guard<std::mutex> scoped_lock(_load_library_lock);
		lib_size = gdmatplot_gnuplot_library_size();
		lib_data = gdmatplot_gnuplot_library_data();
		tmp_lib.resize(lib_size);
		std::memcpy(tmp_lib.ptrw(), lib_data, lib_size);
	}

	fh->store_buffer(tmp_lib);
	fh->flush();

	fh = FileAccess::open(tmp_path, FileAccess::READ);
	if (fh == nullptr) {
		return ERR_LIBGNUPLOT_NOT_WRITTEN;
	}

	if (fh->get_length() != lib_size) {
		GDMATPLOT_ERROR("File size != lib_size: %lu != %u", fh->get_length(), lib_size);

		return ERR_LIBGNUPLOT_NOT_WRITTEN;
	}

#ifdef LINUX_ENABLED
	uint32_t permissions = FileAccess::UNIX_EXECUTE_OWNER;
	permissions |= FileAccess::UNIX_EXECUTE_GROUP;
	permissions |= FileAccess::UNIX_EXECUTE_OTHER;
	permissions |= FileAccess::UNIX_READ_OWNER;
	permissions |= FileAccess::UNIX_READ_GROUP;
	permissions |= FileAccess::UNIX_READ_OTHER;
	permissions |= FileAccess::UNIX_WRITE_OWNER;
	FileAccess::set_unix_permissions(tmp_path, permissions);
#endif

	tmp_path = fh->get_path_absolute();
	int result = _library->load(tmp_path);

	result = _library->init(this);

	int error = DirAccess::remove_absolute(tmp_path);
	if (error == ERR_UNAVAILABLE)
		return ERR_LIBGNUPLOT_NOT_DELETED;

	return result;
}

Variant GDMatPlotNative::run_command(String p_cmd) {
	if (!_library->is_initialized()) {
		GDMATPLOT_ERROR("Call 'load_gnuplot' first");

		return ERR_NOT_INITIALIZED;
	}

	int status = _library->get_status();
	if (status != 0) {
		return status;
	}

	status = _library->run_command(p_cmd);

	return status;
}

Variant GDMatPlotNative::set_dataframe(PackedFloat64Array frame, int p_column_count) {
	if (!_library->is_initialized()) {
		GDMATPLOT_ERROR("Call 'load_gnuplot' first");

		return ERR_NOT_INITIALIZED;
	}

	int64_t size = frame.size();
	if (size % p_column_count) {
		GDMATPLOT_ERROR("Entry count in frame must be divisible by column count");

		return ERR_MISMATCHED_DIMENSION;
	}

	return _library->set_dataframe(frame, p_column_count);
}

Variant GDMatPlotNative::load_dataframe() {
	if (!_library->is_initialized()) {
		GDMATPLOT_ERROR("Call 'load_gnuplot' first");

		return ERR_NOT_INITIALIZED;
	}

	return _library->load_dataframe();
}

Variant GDMatPlotNative::get_version() {
	return GDMATPLOT_VERSION;
}

Variant GDMatPlotNative::get_gnuplot_version() {
	if (!_library->is_initialized()) {
		GDMATPLOT_ERROR("Call 'load_gnuplot' first");

		return String();
	}

	return _library->get_gnuplot_version();
}

void GDMatPlotNative::options() {
	GDMP_MSG("## Called 'options'", 0);
}

void GDMatPlotNative::init() {
	GDMP_MSG("## Called 'init'", 0);

	Rect2 rect(0, 0, _bp.xsize, _bp.ysize);
	draw_rect(rect, Color::hex((_bp.background << 8) | 255));
}

void GDMatPlotNative::reset() {
	GDMP_MSG("## Called 'reset'", 0);
}

void GDMatPlotNative::text() {
	GDMP_MSG("## Called 'text'", 0);
}

int GDMatPlotNative::scale(double x, double y) {
	GDMP_MSG("## Called 'scale(%.04f, %.04f)'", x, y);

	return 0;
}

void GDMatPlotNative::graphics() {
	GDMP_MSG("## Called 'graphics'", 0);
}

void GDMatPlotNative::move(unsigned int x, unsigned int y) {
	_bp.prev_point.x = _bp.X(x);
	_bp.prev_point.y = _bp.Y(y);
}

void GDMatPlotNative::vector(unsigned int x, unsigned int y) {
	if (x != _bp.xlast || y != _bp.ylast) {
		Vector2 from(_bp.xlast, _bp.ylast);
		Vector2 to(_bp.X(x), _bp.Y(y));
		double width = _bp.linewidth * _bp.linewidth_factor;
		if (width < 1.0)
			width = 1.0;
		draw_line(from, to, _bp.get_line_color(), width, true);
	}
}

void GDMatPlotNative::linetype(int type) {
	_bp.linetype = type;
}

void GDMatPlotNative::put_text(unsigned int x, unsigned int y, const char *str) {
	GDMP_MSG("## Called 'put_text(%d, %d, %s)'", x, y, str);

	Ref<Theme> theme = ThemeDB::get_singleton()->get_default_theme();
	if (!theme.is_valid())
		return;

	Ref<Font> font = theme->get_default_font();
	if (!font.is_valid())
		return;

	String _str = str;

	float _x = _bp.X(x);
	float _y = _bp.Y(y);

	float line_height = 0.0;
	constexpr float width = -1;
	constexpr int max_lines = -1;
	const BitField<TextServer::LineBreakFlag> brk_flags = 3;
	const BitField<TextServer::JustificationFlag> justification_flags = 3;
	constexpr TextServer::Direction direction = (TextServer::Direction) 0;
	constexpr TextServer::Orientation orientation = (TextServer::Orientation) 0;
	int font_size = _bp.font.size * _bp.fontscale;
	Vector2 pos(_x, _y);
	Vector2 size = font->get_multiline_string_size(_str, (HorizontalAlignment)_bp.justify,
													width, font_size, max_lines, brk_flags,
													justification_flags, direction, orientation);

	if (_bp.justify == HORIZONTAL_ALIGNMENT_CENTER)
		size.x /= 2.0;
	else if (_bp.justify == HORIZONTAL_ALIGNMENT_LEFT)
		size.x = 0.0;

	pos.x -= size.x;

	draw_set_transform(pos, _bp.text_angle);
	draw_multiline_string(font, Vector2(), _str, (HorizontalAlignment)_bp.justify, width,
							font_size, max_lines, _bp.get_font_color(), brk_flags,
							justification_flags, direction, orientation);
	draw_set_transform(Vector2());
}

int GDMatPlotNative::text_angle(float angle) {
	_bp.text_angle = angle / 180.0 * Math_PI;

	return 0;
}

int GDMatPlotNative::justify_text(int type) {
	_bp.justify = type;

	return 0;
}

void GDMatPlotNative::point(unsigned int x, unsigned int y, int val) {
	Vector2 pos(_bp.X(x), _bp.Y(y));
	draw_circle(pos, _bp.term_pointsize, _bp.get_color());
}

void GDMatPlotNative::arrow(unsigned int x, unsigned int y, unsigned int z,
			unsigned int val, int style) {
	GDMP_MSG("## Called 'arrow(%u, %u, %u, %u, %d)'", x, y, z, val, style);
}

int GDMatPlotNative::set_font(const char *name, double size) {
	_bp.font.size = size;

	if (name) {
		_bp.font.name = name;
		int64_t idx = _bp.font.name.find(",");
		if (idx >= 0) {
			int64_t size = _bp.font.name.substr(idx + 1).to_int();
			if (size > 0)
				_bp.font.size = size;
		}
	}

	return 0;
}

void GDMatPlotNative::pointsize(double size) {
}

void GDMatPlotNative::suspend() {
}

void GDMatPlotNative::resume() {
}

void GDMatPlotNative::fillbox(int style, unsigned int x1, unsigned int y1,
				unsigned int width, unsigned int height) {
	GDMP_MSG("## Called 'fillbox(%d, %u, %u, %u, %u)'", style, x1, y1, width, height);
	Rect2 rect(_bp.X(x1), _bp.Y(y1 + height), _bp.X(width), _bp.X(height));
	draw_rect(rect, _bp.get_fill_color(style));
}

void GDMatPlotNative::linewidth(double p_linewidth) {
	GDMP_MSG("## Called 'linewidth(%.04f)'", p_linewidth);

	_bp.linewidth = p_linewidth;
}

int GDMatPlotNative::make_palette(void *palette) {
	return 0;
}

void GDMatPlotNative::previous_palette() {
}

void GDMatPlotNative::set_color(unsigned int color) {
	GDMP_MSG("## Called 'set_color(%08X)'", color);

	_bp.color = color;
}

void GDMatPlotNative::filled_polygon(int point_count, void *corners, int style) {
	GDMP_MSG("## Called 'filled_polygon(%d, %d)'", point_count, style);

	if (point_count <= 0 || !corners)
		return;

	int *xy = (int *)corners;
	PackedVector2Array points;
	PackedColorArray colors;
	points.resize(point_count);
	colors.resize(point_count);

	for (int i = 0; i < point_count; ++i) {
		int j = 2 * i;
		points[i] = Vector2(_bp.X(xy[j]), _bp.Y(xy[j+1]));
		colors[i] = _bp.get_fill_color(style);
	}

	if (Geometry2D::get_singleton()->triangulate_polygon(points).is_empty())
		return;

	draw_polygon(points, colors);
}

void GDMatPlotNative::image(unsigned int m, unsigned int n, void *imdata,
							void *corners, unsigned int color_mode) {
}

void GDMatPlotNative::enhanced_open(char *fontname, double fontsize,
					double base, char widthflag, char showflag, int overprint) {
}

void GDMatPlotNative::enhanced_flush() {
}

void GDMatPlotNative::enhanced_writec(int c) {
}

void GDMatPlotNative::layer(unsigned int layer) {
}

void GDMatPlotNative::path(int p) {
}

void GDMatPlotNative::hypertext(int type, const char *text) {
	if (text)
		_bp.hypertext = text;
}

void GDMatPlotNative::boxed_text(unsigned int x, unsigned int y, int type) {
	GDMP_MSG("## Called 'boxed_text(%u, %u, %d)'", x, y, type);
}

void GDMatPlotNative::modify_plots(unsigned int operations, int plotno) {
}

void GDMatPlotNative::dashtype(int type, void *custom_dash_pattern) {
}

void GDMatPlotNative::set_xmax(unsigned int val) {
	GDMP_MSG("## Called 'xmax(%u)'", val);
	_bp.xmax = val;
}

void GDMatPlotNative::set_ymax(unsigned int val) {
	GDMP_MSG("## Called 'ymax(%u)'", val);
	_bp.ymax = val;
}

void GDMatPlotNative::set_h_tic(unsigned int val) {
	GDMP_MSG("## Called 'h_tic(%u)'", val);
	_bp.h_tic = val;
}

void GDMatPlotNative::set_v_tic(unsigned int val) {
	GDMP_MSG("## Called 'v_tic(%u)'", val);
	_bp.v_tic = val;
}

void GDMatPlotNative::set_color_mode(unsigned char val) {
	GDMP_MSG("## Called 'color_mode(%u)'", val);
	_bp.color_mode = val;
}

void GDMatPlotNative::set_linetype(int val) {
	GDMP_MSG("## Called 'linetype(%d)'", val);
	_bp.linetype = val;
}

void GDMatPlotNative::set_dashpattern(const char *val) {
	GDMP_MSG("## Called 'dashpattern(%s)'", val);
	if (val)
		_bp.dashpattern = val;
}

void GDMatPlotNative::set_h_char(unsigned int val) {
	GDMP_MSG("## Called 'h_char(%u)'", val);
	_bp.h_char = val;
}

void GDMatPlotNative::set_v_char(unsigned int val) {
	GDMP_MSG("## Called 'v_char(%u)'", val);
	_bp.v_char = val;
}

void GDMatPlotNative::set_gridline(unsigned int val) {
	GDMP_MSG("## Called 'gridline(%u)'", val);
	_bp.gridline = val;
}

void GDMatPlotNative::set_hasgrid(unsigned int val) {
	GDMP_MSG("## Called 'hasgrid(%u)'", val);
	_bp.hasgrid = val;
}

void GDMatPlotNative::set_plotno(unsigned int val) {
	GDMP_MSG("## Called 'plotno(%u)'", val);
	_bp.plotno = val;
}

void GDMatPlotNative::set_fill_pattern(int val) {
	GDMP_MSG("## Called 'fill_pattern(%d)'", val);
	_bp.fill_pattern = val;
}

void GDMatPlotNative::set_fill_pattern_index(unsigned int val) {
	GDMP_MSG("## Called 'fill_pattern_index(%u)'", val);
	_bp.fill_pattern_index = val;
}

void GDMatPlotNative::set_rgb(unsigned int val) {
	GDMP_MSG("## Called 'rgb(%06X)'", val);
	_bp.rgb = (val << 8) | 255;
}

void GDMatPlotNative::set_patterncolor(unsigned int *patterncolor) {
	GDMP_MSG("## Called 'patterncolor(ptr)'", 0);
	std::memcpy(_bp.patterncolor, patterncolor, sizeof(_bp.patterncolor));
}

void GDMatPlotNative::set_group_filled_is_open(unsigned char val) {
	GDMP_MSG("## Called 'group_filled_is_open(%uc)'", val);
	_bp.group_filled_is_open = val;
}

void GDMatPlotNative::set_in_textbox(unsigned char val) {
	GDMP_MSG("## Called 'in_textbox(%uc)'", val);
	_bp.in_textbox = val;
}

void GDMatPlotNative::set_xsize(unsigned int val) {
	GDMP_MSG("## Called 'xsize(%u)'", val);
	_bp.xsize = (float)val * BackendParams::GDMP_SCALE;
}

void GDMatPlotNative::set_ysize(unsigned int val) {
	GDMP_MSG("## Called 'ysize(%u)'", val);
	_bp.ysize = (float)val * BackendParams::GDMP_SCALE;
}

void GDMatPlotNative::set_xlast(unsigned int val) {
	_bp.xlast = _bp.X(val);
}

void GDMatPlotNative::set_ylast(unsigned int val) {
	_bp.ylast = _bp.Y(val);
}

void GDMatPlotNative::set_linecap(int val) {
	GDMP_MSG("## Called 'linecap(%d)'", val);
	_bp.linecap = val;
}

void GDMatPlotNative::set_group_is_open(unsigned char val) {
	GDMP_MSG("## Called 'group_is_open(%uc)'", val);
	_bp.group_is_open = val;
}

void GDMatPlotNative::set_path_is_open(unsigned char val) {
	GDMP_MSG("## Called 'path_is_open(%uc)'", val);
	_bp.path_is_open = val;
}

void GDMatPlotNative::set_fontscale(double val) {
	GDMP_MSG("## Called 'fontscale(%.02f)'", val);
	_bp.fontscale = val;
}

void GDMatPlotNative::set_dashlength(double val) {
	GDMP_MSG("## Called 'dashlength(%.02f)'", val);
	_bp.dashlength = val;
}

void GDMatPlotNative::set_name(const char *val) {
	GDMP_MSG("## Called 'name(%s)'", val);
	if (val)
		_bp.name = val;
}

void GDMatPlotNative::set_linewidth_factor(double val) {
	GDMP_MSG("## Called 'linewidth(%.02f)'", val);
	_bp.linewidth_factor = 0.5 * val;
}

void GDMatPlotNative::set_background(int val) {
	GDMP_MSG("## Called 'background(%d)'", val);
	_bp.background = val;
}

void GDMatPlotNative::set_linecolor(const char *val) {
	GDMP_MSG("## Called 'linecolor(%s)'", val);
	if (val) {
		unsigned int color = 0;

		String name = val;
		Color c;
		PackedStringArray rgb_vals = name.split(",", false);
		if (rgb_vals.size() == 3) {
			int red = rgb_vals[0].to_int();
			int green = rgb_vals[1].to_int();
			int blue = rgb_vals[2].to_int();
			unsigned int color = BackendParams::DEFAULT_COLOR;
			color |= (blue << 8) | (green << 16) | (red << 24);

			c = Color::hex(color);
		} else {
			c = Color::named(name);
		}

		_bp.linecolor = c.to_rgba32();
	}
}

void GDMatPlotNative::set_alpha(double val) {
	_bp.alpha = val;
}

void GDMatPlotNative::set_term_pointsize(double val) {
	_bp.term_pointsize = val;
}

void GDMatPlotNative::set_stroke_width(double val) {
	_bp.term_pointsize = val;
}

void GDMatPlotNative::set_pen(unsigned int index, unsigned int color, double width) {
	if (index >= sizeof(_bp.pens) / sizeof(_bp.pens[0])) {
		_bp.pen_index = index >> 8;

		return;
	}

	BackendParams::Pen &pen = _bp.pens[index];
	pen.color = color;
	pen.width = width;
}
