/**************************************************************************/
/*  gdmatplot.h                                                           */
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

#ifndef GDMATPLOT_H_
#define GDMATPLOT_H_

#include <string.h>

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/geometry2d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/theme_db.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "gdmatplot_backend.h"
#include "utils.h"

// #define GDMP_MSG(fmt, ...) GDMATPLOT_V_DEBUG(fmt, __VA_ARGS__)
#define GDMP_MSG(fmt, ...) \
	do {                   \
	} while (0)
#define GDMP_MSG_V(fmt, ...) GDMATPLOT_V_DEBUG(fmt, __VA_ARGS__)

namespace godot {

class GDMatPlotNative : public Node2D {
	GDCLASS(GDMatPlotNative, Node2D)

protected:
	enum ErrorCodes {
		ERR_GENERAL = std::numeric_limits<short>::lowest(),
		ERR_LIBGNUPLOT_NOT_FOUND,
		ERR_LIBGNUPLOT_NOT_READ,
		ERR_LIBGNUPLOT_NOT_WRITTEN,
		ERR_LIBGNUPLOT_NOT_DELETED,
		ERR_FILE_ACCESS,
		ERR_INVALID_TYPE,
		ERR_INVALID_FIGURE,
		ERR_LIBGNUPLOT_NOT_INITIALIZED,
		ERR_MISMATCHED_DIMENSION,
		ERR_NOT_INITIALIZED,
	};

	struct BackendParams {
		static constexpr unsigned int DEFAULT_COLOR = 0x000000FFu;
		static constexpr float GDMP_SCALE = 0.01;

		enum colortype {
			TC_DEFAULT = 0, /* Use default color, set separately */
			TC_LT = 1, /* Use the color of linetype <n> */
			TC_LINESTYLE = 2, /* Use the color of line style <n> */
			TC_RGB = 3, /* Explicit RGB triple provided by user */
			TC_CB = 4, /* "palette cb <value>" */
			TC_FRAC = 5, /* "palette frac <value> */
			TC_Z = 6, /* "palette z" */
			TC_VARIABLE = 7, /* only used for "tc", never "lc" */
			TC_COLORMAP = 8 /* "palette colormap" */
		};

		enum t_fillstyle {
			FS_EMPTY,
			FS_SOLID,
			FS_PATTERN,
			FS_DEFAULT,
			FS_TRANSPARENT_SOLID,
			FS_TRANSPARENT_PATTERN,
		};

		struct Font {
			String name;
			double size{ 12.0 };
		};

		struct Pen {
			double width{ 2.0 };
			unsigned int color{ DEFAULT_COLOR };
		};

		Color get_color() {
			switch (color_mode) {
				case TC_LT:
					return Color::hex(linecolor);
				case TC_RGB:
					return Color::hex(color);
				default:
					break;
			}

			return Color::hex(DEFAULT_COLOR);
		}

		Color get_line_color() {
			switch (color_mode) {
				case TC_LT:
					return Color::hex(linecolor);
				case TC_RGB:
					return Color::hex(color);
				default:
					break;
			}

			return Color::hex(pens[pen_index].color);
		}

		Color get_font_color() {
			switch (color_mode) {
				case TC_LT:
					return Color::hex(linecolor);
				case TC_RGB:
					return Color::hex(color);
				default:
					break;
			}

			return Color::hex(pens[pen_index].color);
		}

		Color get_fill_color(int style) {
			int fillpar = style >> 4;
			style &= 0x0F;

			Color c = get_color();

			if (alpha != 1.0)
				c.a = alpha;
			else if (fillpar >= 0 && fillpar < 100)
				c.a = (float)fillpar * 0.01;

			switch (style) {
				case FS_SOLID:
					return c;
				default:
					break;
			}

			return Color::hex(DEFAULT_COLOR);
		}

		float X(float x) {
			return x * GDMP_SCALE;
		}

		float Y(float y) {
			return ysize - y * GDMP_SCALE;
		}

		int Xi(float x) {
			return (int)(x * GDMP_SCALE);
		}

		int Yi(float y) {
			return (int)(ysize - y * GDMP_SCALE);
		}

		Pen pens[16];
		Font font;
		String hypertext;
		String dashpattern;
		String name;
		Vector2 prev_point;
		double linewidth{ 2.0 };
		double fontscale{ 1.0 };
		double dashlength{ 2.0 };
		double linewidth_factor{ 1.0 };
		double alpha{ 1.0 };
		double term_pointsize{ 1.0 };
		double stroke_width{ 2.0 };
		float text_angle{ 0.0 };
		int justify{};
		unsigned int color{ 0x000000FFu };
		unsigned int xmax{};
		unsigned int ymax{};
		unsigned int h_tic{};
		unsigned int v_tic{};
		int linetype{};
		unsigned int h_char{};
		unsigned int v_char{};
		unsigned int gridline{};
		unsigned int hasgrid{};
		unsigned int plotno{};
		int fill_pattern{};
		unsigned int fill_pattern_index{};
		unsigned int rgb{};
		unsigned int patterncolor[8];
		float xsize{};
		float ysize{};
		unsigned int xlast{};
		unsigned int ylast{};
		int linecap{};
		int background{};
		int linecolor{ 0x000000FFu };
		int pen_index{};

		unsigned char color_mode{};
		unsigned char group_filled_is_open{};
		unsigned char in_textbox{};
		unsigned char group_is_open{};
		unsigned char path_is_open{};
	};

	BackendParams _bp;
	Ref<GDMatPlotBackend> _library;

	static void _bind_methods();

public:
	GDMatPlotNative();

	Variant load_gnuplot(String p_path);
	Variant run_command(String p_cmd);
	Variant set_dataframe(PackedFloat64Array frame, int p_column_count);
	Variant load_dataframe();
	Variant get_version();
	Variant get_gnuplot_version();

	void options();
	void init();
	void reset();
	void text();
	int scale(double x, double y);
	void graphics();
	void move(unsigned int x, unsigned int y);
	void vector(unsigned int x, unsigned int y);
	void linetype(int type);
	void put_text(unsigned int x, unsigned int y, const char *str);
	int text_angle(float angle);
	int justify_text(int type);
	void point(unsigned int x, unsigned int y, int val);
	void arrow(unsigned int x, unsigned int y, unsigned int z,
			unsigned int val, int style);
	int set_font(const char *name, double size);
	void pointsize(double size);
	void suspend();
	void resume();
	void fillbox(int style, unsigned int x1, unsigned int y1,
			unsigned int width, unsigned int height);
	void linewidth(double p_linewidth);
	int make_palette(void *palette);
	void previous_palette();
	void set_color(unsigned int color);
	void filled_polygon(int point_count, void *corners, int style);
	void image(unsigned int m, unsigned int n, void *imdata,
			void *corners, unsigned int color_mode);
	void enhanced_open(char *fontname, double fontsize,
			double base, char widthflag, char showflag, int overprint);
	void enhanced_flush();
	void enhanced_writec(int c);
	void layer(unsigned int layer);
	void path(int p);
	void hypertext(int type, const char *text);
	void boxed_text(unsigned int x, unsigned int y, int type);
	void modify_plots(unsigned int operations, int plotno);
	void dashtype(int type, void *custom_dash_pattern);
	void set_xmax(unsigned int val);
	void set_ymax(unsigned int val);
	void set_h_tic(unsigned int val);
	void set_v_tic(unsigned int val);
	void set_color_mode(unsigned char val);
	void set_linetype(int val);
	void set_dashpattern(const char *val);
	void set_h_char(unsigned int val);
	void set_v_char(unsigned int val);
	void set_gridline(unsigned int val);
	void set_hasgrid(unsigned int val);
	void set_plotno(unsigned int val);
	void set_fill_pattern(int val);
	void set_fill_pattern_index(unsigned int val);
	void set_rgb(unsigned int val);
	void set_patterncolor(unsigned int *patterncolor);
	void set_group_filled_is_open(unsigned char val);
	void set_in_textbox(unsigned char val);
	void set_xsize(unsigned int val);
	void set_ysize(unsigned int val);
	void set_xlast(unsigned int val);
	void set_ylast(unsigned int val);
	void set_linecap(int val);
	void set_group_is_open(unsigned char val);
	void set_path_is_open(unsigned char val);
	void set_fontscale(double val);
	void set_dashlength(double val);
	void set_name(const char *val);
	void set_linewidth_factor(double val);
	void set_background(int val);
	void set_linecolor(const char *val);
	void set_alpha(double val);
	void set_term_pointsize(double val);
	void set_stroke_width(double val);
	void set_pen(unsigned int index, unsigned int color, double width);
};

} //namespace godot

#endif // GDMATPLOT_H_
