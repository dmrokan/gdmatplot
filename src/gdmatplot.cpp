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
#include <limits>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/geometry2d.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/theme_db.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>

#include "gdmatplot_gnuplot_library.h"

using namespace godot;

static constexpr int GNUPLOT_RENDERER_BASE_DELAY = 16;
static constexpr int GNUPLOT_RENDERER_DEFAULT_LOOP_PERIOD = 1000;
static std::mutex _load_library_lock;

void GDMatPlotNative::_bind_methods() {
	// Floats must have 32-bit width for encoding/decoding of draw commands.
	// Otherwise, build will fail. Platform compatibility is left as future work.
	static_assert(std::numeric_limits<float>::is_iec559);

	ClassDB::bind_method(D_METHOD("draw_plot"), &GDMatPlotNative::_draw);
	ClassDB::bind_method(D_METHOD("load_gnuplot", "p_path"), &GDMatPlotNative::load_gnuplot,
						 DEFVAL(String(GDMATPLOT_DEFAULT_LIBGNUPLOT_PATH)));
	ClassDB::bind_method(D_METHOD("run_command", "p_cmd"), &GDMatPlotNative::run_command);
	ClassDB::bind_method(D_METHOD("set_dataframe", "p_data", "p_column_count"),
						 &GDMatPlotNative::set_dataframe);
	ClassDB::bind_method(D_METHOD("load_dataframe"), &GDMatPlotNative::load_dataframe);
	ClassDB::bind_method(D_METHOD("start_renderer", "p_loop_func"),
						 &GDMatPlotNative::start_renderer);
	ClassDB::bind_method(D_METHOD("set_rendering_period", "p_loop_period"),
						 &GDMatPlotNative::set_rendering_period);
	ClassDB::bind_method(D_METHOD("stop_renderer"), &GDMatPlotNative::stop_renderer);
	ClassDB::bind_method(D_METHOD("get_version"), &GDMatPlotNative::get_version);
	ClassDB::bind_method(D_METHOD("get_gnuplot_version"), &GDMatPlotNative::get_gnuplot_version);

	ClassDB::bind_method(D_METHOD("set_background_transparency", "p_alpha"),
	&GDMatPlotNative::set_background_transparency);
	ClassDB::bind_method(D_METHOD("get_background_transparency"),
						 &GDMatPlotNative::get_background_transparency);
	ClassDB::add_property("GDMatPlotNative",
						  PropertyInfo(Variant::FLOAT, "transparency", PROPERTY_HINT_RANGE, "0,1,0.01"),
						  "set_background_transparency", "get_background_transparency");

	ClassDB::bind_method(D_METHOD("set_antialiased", "p_antialiased"), &GDMatPlotNative::set_antialiased);
	ClassDB::bind_method(D_METHOD("get_antialiased"), &GDMatPlotNative::get_antialiased);
	ClassDB::add_property("GDMatPlotNative", PropertyInfo(Variant::BOOL, "antialiasing"),
						  "set_antialiased", "get_antialiased");
}

GDMatPlotNative::GDMatPlotNative() {
	_library.instantiate();
	_get_set_stage_lock.instantiate();
}

GDMatPlotNative::~GDMatPlotNative() {
	stop_renderer();
	_library->unload();
}

void GDMatPlotNative::_set_stage(int p_stage) {
	MutexLock lock(**_get_set_stage_lock);
	_stage = p_stage;
}

int GDMatPlotNative::_get_stage() {
	MutexLock lock(**_get_set_stage_lock);
	return _stage;
}

void GDMatPlotNative::_draw_plot() {
	_set_stage(GODOT_STAGE);

	if (_encoded_drawings.size() > 0) {
		for (auto iter = _encoded_drawings.begin(); iter != _encoded_drawings.end(); ++iter) {
			EncodedDrawing &fd = *iter;
			fd.decode(this);
		}

		_encoded_drawings.clear();
	}

	_set_stage(GNUPLOT_STAGE);

	if (_gnuplot_render_thread_func != nullptr)
		_gnuplot_render_thread_func->trigger_render();
}

void GDMatPlotNative::_draw() {
	_draw_plot();
}

void GDMatPlotNative::_draw_path_points(bool is_open) {
	GDMP_MSG("## Called 'path_is_open(%uc)'", val);

	if (_path_points.size() < 2) {
		return;
	}

	double width = _bp.linewidth * _bp.linewidth_factor;
	if (width < 1.0)
		width = 1.0;

	EncodedDrawing ed;
	PackedVector2Array path_points(_path_points);
	ed.encode_path_is_open(path_points, true, width, _bp.get_line_color());
	_encoded_drawings.push_back(ed);
	_path_points.clear();
}

void GDMatPlotNative::set_background_transparency(float p_transparency) {
	_background_alpha = p_transparency;
}

float GDMatPlotNative::get_background_transparency() const {
	return _background_alpha;
}

void GDMatPlotNative::set_antialiased(bool p_antialiased) {
	_antialiased = p_antialiased;
}

bool GDMatPlotNative::get_antialiased() const {
	return _antialiased;
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
	int error = _library->load(tmp_path);
	if (!error) {
		error = _library->init(this);
	}

	int error_tmp = DirAccess::remove_absolute(tmp_path);
	if (error_tmp == ERR_UNAVAILABLE) {
		if (error)
			return error;

		return ERR_LIBGNUPLOT_NOT_DELETED;
	}

	return error;
}

Variant GDMatPlotNative::run_command(String p_cmd) {
	if (!_library->is_loaded()) {
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
	if (!_library->is_loaded()) {
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
	if (!_library->is_loaded()) {
		GDMATPLOT_ERROR("Call 'load_gnuplot' first");

		return ERR_NOT_INITIALIZED;
	}

	return _library->load_dataframe();
}

Variant GDMatPlotNative::start_renderer(Callable p_loop_func) {
	stop_renderer();

	// It is automatically freed on thread cleanup.
	_gnuplot_render_thread_func = memnew(GDMatPlotGNUPlotRenderer);
	if (_gnuplot_render_thread_func == nullptr)
		return ERR_GENERAL;

	_gnuplot_render_thread_func->set_loop_func(p_loop_func);
	Callable c(_gnuplot_render_thread_func);

	_gnuplot_render_thread.instantiate();
	Error error = _gnuplot_render_thread->start(c);
	if (error) {
		GDMATPLOT_ERROR("Could not start GNUPlot renderer, exits with code %d", error);

		return ERR_GNUPLOT_RENDERER_START;
	}

	return 0;
}

void GDMatPlotNative::set_rendering_period(int64_t p_period) {
	if (_gnuplot_render_thread_func == nullptr)
		return;

	_gnuplot_render_thread_func->set_loop_period(p_period);
}

void GDMatPlotNative::stop_renderer() {
	if (_gnuplot_render_thread_func == nullptr)
		return;

	_gnuplot_render_thread_func->terminate();
	if (_gnuplot_render_thread->is_alive()) {
		_gnuplot_render_thread->wait_to_finish();
	}

	_gnuplot_render_thread.unref();
	_gnuplot_render_thread_func = nullptr;
}

Variant GDMatPlotNative::get_version() {
	return GDMATPLOT_VERSION;
}

Variant GDMatPlotNative::get_gnuplot_version() {
	if (!_library->is_loaded()) {
		GDMATPLOT_ERROR("Call 'load_gnuplot' first");

		return String();
	}

	return _library->get_gnuplot_version();
}

/*
 ********************************* GNUPlot renderer *******************************
 */

void GDMatPlotGNUPlotRenderer::call(const Variant **p_arguments, int p_argcount, Variant &r_return_value,
									GDExtensionCallError &r_call_error) const {
	while (!_is_terminated()) {
		uint64_t loop_start = Time::get_singleton()->get_ticks_msec();

		while (!_sem->try_wait()) {
			if (_is_terminated())
				goto end_of_thread;

			OS::get_singleton()->delay_msec(GNUPLOT_RENDERER_BASE_DELAY);
		}

		_loop_func.call_deferred();

		int64_t dt = Time::get_singleton()->get_ticks_msec() - loop_start;
		int64_t loop_period = get_loop_period();
		while (dt < loop_period && !_is_terminated()) {
			int64_t delay = loop_period - dt;
			if (delay > GNUPLOT_RENDERER_BASE_DELAY)
				delay = GNUPLOT_RENDERER_BASE_DELAY;
			OS::get_singleton()->delay_msec(delay);
			dt = Time::get_singleton()->get_ticks_msec() - loop_start;
			loop_period = get_loop_period();
		}
	}

end_of_thread:

	r_call_error.error = GDEXTENSION_CALL_OK;
}

/*
 ********************************* EncodedDrawing *******************************
 */

GDMatPlotNative::EncodedDrawing::EncodedDrawing() {
	draw_command.resize(16);
}

GDMatPlotNative::EncodedDrawing::EncodedDrawing(const EncodedDrawing &other) {
	if (this == &other)
		return;

	draw_command = other.draw_command;
	path_points = other.path_points;
	path_color = other.path_color;
	path_width = other.path_width;
	_size = other._size;
	type = other.type;
}

void GDMatPlotNative::EncodedDrawing::operator =(const EncodedDrawing &other) {
	if (this == &other)
		return;

	draw_command = other.draw_command;
	path_points = other.path_points;
	path_color = other.path_color;
	path_width = other.path_width;
	_size = other._size;
	type = other.type;
}

void GDMatPlotNative::EncodedDrawing::resize() {
	PackedByteArray increment;
	increment.resize(draw_command.size());
	draw_command.append_array(increment);
}

void GDMatPlotNative::EncodedDrawing::encode_color(const Color &color) {
	unsigned int tmp = color.to_rgba32();
	encode(tmp);
}

void GDMatPlotNative::EncodedDrawing::encode_draw_rect(float x, float y, float width,
													   float height, const Color &c) {
	encode(x);
	encode(y);
	encode(width);
	encode(height);
	encode_color(c);
	type = RECT;
}

void GDMatPlotNative::EncodedDrawing::decode_draw_rect(GDMatPlotNative *self) {
	const float *tmpf = (const float *)(draw_command.ptr());
	float x = *tmpf++;
	float y = *tmpf++;
	float width = *tmpf++;
	float height = *tmpf++;
	// cppcheck-suppress invalidPointerCast
	const uint32_t *tmpi = (const uint32_t *)(tmpf);
	Rect2 rect(x, y, width, height);
	self->draw_rect(rect, Color::hex(*tmpi));
}

void GDMatPlotNative::EncodedDrawing::encode_draw_line(float x1, float y1, float x2, float y2,
													   float width, const Color &c) {
	encode(x1);
	encode(y1);
	encode(x2);
	encode(y2);
	encode(width);
	encode_color(c);
	type = LINE;
}

void GDMatPlotNative::EncodedDrawing::decode_draw_line(GDMatPlotNative *self) {
	const float *tmpf = (const float *)(draw_command.ptr());
	float x1 = *tmpf++;
	float y1 = *tmpf++;
	float x2 = *tmpf++;
	float y2 = *tmpf++;
	float width = *tmpf++;
	// cppcheck-suppress invalidPointerCast
	const uint32_t *tmpi = (const uint32_t *)(tmpf);
	Vector2 from(x1, y1);
	Vector2 to(x2, y2);
	self->draw_line(from, to, Color::hex(*tmpi), width, self->get_antialiased());
}

void GDMatPlotNative::EncodedDrawing::encode_put_text(float x, float y, float text_angle, int font_size,
													  int justify, const Color &c, const String &s) {
	PackedByteArray str_buf = s.to_utf8_buffer();
	draw_command.resize(4);
	draw_command.encode_u32(0, str_buf.size());
	draw_command.append_array(str_buf);
	_size = draw_command.size();
	encode(x);
	encode(y);
	encode(text_angle);
	encode(font_size);
	encode(justify);
	encode_color(c);
	type = TEXT;
}

void GDMatPlotNative::EncodedDrawing::decode_put_text(GDMatPlotNative *self) {
	unsigned int size = draw_command.decode_u32(0);
	const unsigned int offset = sizeof(size);
	String str = draw_command.slice(offset, offset + size).get_string_from_utf8();
	const float *tmpf = (const float *)(draw_command.ptr() + size + offset);
	float x = *tmpf++;
	float y = *tmpf++;
	float text_angle_ = *tmpf++;
	// cppcheck-suppress invalidPointerCast
	const int32_t *tmp = (const int32_t *)(tmpf);
	int font_size = *tmp++;
	int justify = *tmp++;
	const uint32_t *tmpi = (const uint32_t *)(tmp);
	uint32_t color = *tmpi++;

	Ref<Theme> theme = ThemeDB::get_singleton()->get_default_theme();
	if (!theme.is_valid())
		return;

	Ref<Font> font = theme->get_default_font();
	if (!font.is_valid())
		return;

	constexpr float width = -1;
	constexpr int max_lines = -1;
	const BitField<TextServer::LineBreakFlag> brk_flags = 3;
	const BitField<TextServer::JustificationFlag> justification_flags = 3;
	constexpr TextServer::Direction direction = (TextServer::Direction) 0;
	constexpr TextServer::Orientation orientation = (TextServer::Orientation) 0;
	Vector2 pos(x, y);
	Vector2 str_size = font->get_multiline_string_size(str, (HorizontalAlignment)justify,
														width, font_size, max_lines, brk_flags,
														justification_flags, direction, orientation);

	if (justify == HORIZONTAL_ALIGNMENT_CENTER)
		str_size.x /= 2.0;
	else if (justify == HORIZONTAL_ALIGNMENT_LEFT)
		str_size.x = 0.0;

	pos.x -= str_size.x;

	self->draw_set_transform(pos, text_angle_);
	self->draw_multiline_string(font, Vector2(), str, (HorizontalAlignment)justify, width,
								font_size, max_lines, Color::hex(color), brk_flags,
								justification_flags, direction, orientation);
	self->draw_set_transform(Vector2());
}

void GDMatPlotNative::EncodedDrawing::encode_point(float x, float y, float size, const Color &c) {
	encode(x);
	encode(y);
	encode(size);
	encode_color(c);
	type = POINT;
}

void GDMatPlotNative::EncodedDrawing::decode_point(GDMatPlotNative *self) {
	const float *tmpf = (const float *)(draw_command.ptr());
	float x = *tmpf++;
	float y = *tmpf++;
	float size = *tmpf++;
	// cppcheck-suppress invalidPointerCast
	const uint32_t *tmpi = (const uint32_t *)(tmpf);
	Vector2 pos(x, y);
	self->draw_circle(pos, size, Color::hex(*tmpi));
}

void GDMatPlotNative::EncodedDrawing::encode_filled_polygon(int point_count, const int *corners,
															const Color &c) {
	encode_color(c);
	encode(point_count);

	for (int i = 0; i < point_count; ++i) {
		int j = 2 * i;
		encode(corners[j]);
		encode(corners[j+1]);
	}

	type = POLYGON;
}

void GDMatPlotNative::EncodedDrawing::decode_filled_polygon(GDMatPlotNative *self) {
	const uint32_t *tmp_color = (const uint32_t *)(draw_command.ptr());
	uint32_t color = *tmp_color++;
	const int32_t *tmpi = (const int32_t *)(tmp_color);

	int point_count = *tmpi++;
	PackedVector2Array points;
	PackedColorArray colors;
	points.resize(point_count);
	colors.resize(point_count);

	for (int i = 0; i < point_count; ++i) {
		int x = *tmpi++;
		int y = *tmpi++;
		points[i] = Vector2(self->_bp.X(x), self->_bp.Y(y));
		colors[i] = Color::hex(color);
	}

	if (Geometry2D::get_singleton()->triangulate_polygon(points).is_empty())
		return;

	self->draw_polygon(points, colors);
}

void GDMatPlotNative::EncodedDrawing::encode_init(float x, float y, float width,
												  float height, const Color &c) {
	encode_draw_rect(x, y, width, height, c);
	type = INIT;
}

void GDMatPlotNative::EncodedDrawing::decode_init(GDMatPlotNative *self) {
	decode_draw_rect(self);
}

void GDMatPlotNative::EncodedDrawing::encode_vector(float x1, float y1, float x2, float y2,
													float width, const Color &c) {
	encode_draw_line(x1, y1, x2, y2, width, c);
	type = VECTOR;
}

void GDMatPlotNative::EncodedDrawing::decode_vector(GDMatPlotNative *self) {
	decode_draw_line(self);
}

void GDMatPlotNative::EncodedDrawing::encode_fillbox(float x, float y, float width,
													 float height, const Color &c) {
	encode_draw_rect(x, y, width, height, c);
	type = FILLBOX;
}

void GDMatPlotNative::EncodedDrawing::decode_fillbox(GDMatPlotNative *self) {
	decode_draw_rect(self);
}

void GDMatPlotNative::EncodedDrawing::encode_path_is_open(const PackedVector2Array &points, unsigned int is_open,
														  float width, const Color &c) {
	encode(is_open);
	encode_color(c);
	encode(width);
	path_points = points;
	type = PATH_OPEN;
}

void GDMatPlotNative::EncodedDrawing::decode_path_is_open(GDMatPlotNative *self) {
	const uint32_t *tmpi = (const uint32_t *)(draw_command.ptr());
	// cppcheck-suppress unreadVariable
	uint32_t is_open = *tmpi++;
	uint32_t color = *tmpi++;
	// cppcheck-suppress invalidPointerCast
	const float *tmpf = (float *)tmpi;
	float width = *tmpf++;

	self->draw_polyline(path_points, Color::hex(color), width, self->get_antialiased());
}

void GDMatPlotNative::EncodedDrawing::decode(GDMatPlotNative *self) {
	switch (type) {
	case INIT:
		decode_init(self);
		break;
	case VECTOR:
		decode_vector(self);
		break;
	case FILLBOX:
		decode_fillbox(self);
		break;
	case POINT:
		decode_point(self);
		break;
	case POLYGON:
		decode_filled_polygon(self);
		break;
	case RECT:
		decode_draw_rect(self);
		break;
	case LINE:
		decode_draw_line(self);
		break;
	case TEXT:
		decode_put_text(self);
		break;
	case PATH_OPEN:
		decode_path_is_open(self);
	default:
		break;
	}
}

/*
 ******************** GNUPlot GDMP Terminal methods endpoints ********************
 */

void GDMatPlotNative::options() {
	GDMP_MSG("## Called 'options'", 0);
}

void GDMatPlotNative::init() {
	GDMP_MSG("## Called 'init'", 0);

	if (_get_stage() != GNUPLOT_STAGE)
		return;

	int alpha = Math::round((float)(255.0 * _background_alpha));
	alpha = alpha > 255 ? 255 : (alpha < 0 ? 0 : alpha);
	EncodedDrawing fd;
	fd.encode_init(0, 0, _bp.xsize, _bp.ysize, Color::hex((_bp.background << 8) | alpha));
	_encoded_drawings.push_back(fd);
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

	_draw_path_points(false);
}

void GDMatPlotNative::vector(unsigned int x, unsigned int y) {
	if (_get_stage() != GNUPLOT_STAGE)
		return;

	if (_path_points.size() == 0)
		_path_points.append(Vector2(_bp.xlast, _bp.ylast));

	_path_points.append(Vector2(_bp.X(x), _bp.Y(y)));
}

void GDMatPlotNative::linetype(int type) {
	_bp.linetype = type;
}

void GDMatPlotNative::put_text(unsigned int x, unsigned int y, const char *str) {
	GDMP_MSG("## Called 'put_text(%d, %d, %s)'", x, y, str);

	if (_get_stage() != GNUPLOT_STAGE)
		return;

	if (!str)
		return;

	String _str = str;
	int font_size = _bp.font.size * _bp.fontscale;
	EncodedDrawing fd;
	fd.encode_put_text(_bp.X(x), _bp.Y(y), _bp.text_angle, font_size, _bp.justify,
						_bp.get_font_color(), _str);
	_encoded_drawings.push_back(fd);
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
	if (_get_stage() != GNUPLOT_STAGE)
		return;

	EncodedDrawing fd;
	fd.encode_point(_bp.X(x), _bp.Y(y), _bp.term_pointsize, _bp.get_color());
	_encoded_drawings.push_back(fd);
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
			int64_t tmp = _bp.font.name.substr(idx + 1).to_int();
			if (tmp > 0)
				_bp.font.size = tmp;
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

	if (_get_stage() != GNUPLOT_STAGE)
		return;

	EncodedDrawing fd;
	fd.encode_fillbox(_bp.X(x1), _bp.Y(y1 + height), _bp.X(width), _bp.X(height),
					  _bp.get_fill_color(style));
	_encoded_drawings.push_back(fd);
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

	if (_get_stage() != GNUPLOT_STAGE)
		return;

	if (point_count <= 0 || !corners)
		return;

	const int *xy = (int *)corners;

	EncodedDrawing fd;
	fd.encode_filled_polygon(point_count, xy, _bp.get_fill_color(style));
	_encoded_drawings.push_back(fd);
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

	if (val)
		return;

	_draw_path_points(val);
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
	_bp.linewidth_factor = val;
}

void GDMatPlotNative::set_background(int val) {
	GDMP_MSG("## Called 'background(%d)'", val);
	_bp.background = val;
}

void GDMatPlotNative::set_linecolor(const char *val) {
	GDMP_MSG("## Called 'linecolor(%s)'", val);
	if (val) {
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
