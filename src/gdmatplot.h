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

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/mutex_lock.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "gdmatplot_backend.h"
#include "utils.h"

#if defined(LINUX_ENABLED)
#define GDMATPLOT_DEFAULT_LIBGNUPLOT_PATH "user://libgnuplot.so"
#elif defined(WINDOWS_ENABLED)
#define GDMATPLOT_DEFAULT_LIBGNUPLOT_PATH "user://libgnuplot.dll"
#endif

#ifdef GDMP_ENABLE_DEBUG_MESSAGE
#define GDMP_MSG(fmt, ...) GDMATPLOT_V_DEBUG(fmt, __VA_ARGS__)
#else
#define GDMP_MSG(fmt, ...) \
	do {                   \
	} while (0)
#endif

namespace godot {

class GDMatPlotGNUPlotRenderer : public CallableCustom {
	Callable _loop_func;
	Ref<Semaphore> _sem;
	Ref<Mutex> _lock;
	int64_t _loop_period{ 1000 }; // Period in ms
	bool _terminate{ false };
	bool _paused{ false };

	bool _is_terminated() const {
		MutexLock lock(**_lock);
		return _terminate;
	}

public:
	GDMatPlotGNUPlotRenderer() {
		_lock.instantiate();
		_sem.instantiate();
		_sem->post();
		GDMATPLOT_DEBUG("## Created: GDMatPlotGNUPlotRenderer instance: %p", this);
	}

	~GDMatPlotGNUPlotRenderer() {
		terminate();
		_sem->post();
		GDMATPLOT_DEBUG("## Deleted: GDMatPlotGNUPlotRenderer instance: %p", this);
	}

	static bool compare_equal_func(const CallableCustom *p_a, const CallableCustom *p_b) {
		return p_a == p_b;
	}

	static bool compare_less_func(const CallableCustom *p_a, const CallableCustom *p_b) {
		return (void *)p_a < (void *)p_b;
	}

	uint32_t hash() const override {
		return 0x999999FFu;
	}

	String get_as_text() const override {
		return "<GDMatPlotGNUPlotRenderer>";
	}

	CompareEqualFunc get_compare_equal_func() const override {
		return &GDMatPlotGNUPlotRenderer::compare_equal_func;
	}

	CompareLessFunc get_compare_less_func() const override {
		return &GDMatPlotGNUPlotRenderer::compare_less_func;
	}

	bool is_valid() const override {
		return true;
	}

	ObjectID get_object() const override {
		return ObjectID();
	}

	void set_loop_period(int64_t period) {
		MutexLock lock(**_lock);
		_loop_period = period;
	}

	int64_t get_loop_period() const {
		MutexLock lock(**_lock);
		return _loop_period;
	}

	void terminate() {
		MutexLock lock(**_lock);
		_terminate = true;
	}

	void set_loop_func(const Callable &c) {
		_loop_func = c;
	}

	void trigger_render() {
		_sem->post();
	}

	void call(const Variant **p_arguments, int p_argcount, Variant &r_return_value,
			GDExtensionCallError &r_call_error) const override;
};

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
		ERR_GNUPLOT_RENDERER_START,
	};

	enum DrawStage {
		UNDEFINED_STAGE = 0,
		GNUPLOT_STAGE,
		GODOT_STAGE,
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

	struct EncodedDrawing {
		enum Type {
			NONE = 0,
			LINE,
			RECT,
			POLYGON,
			TEXT,
			INIT,
			VECTOR,
			POINT,
			FILLBOX,
			PATH_OPEN,
		};

		PackedByteArray draw_command;
		PackedVector2Array path_points;
		size_t _size{};
		unsigned int path_color{};
		float path_width{};
		int type{ NONE };

		EncodedDrawing();
		EncodedDrawing(const EncodedDrawing &other);
		void operator=(const EncodedDrawing &other);

		void resize();

		template <typename T, int S = sizeof(T)>
		void encode(T &d) {
			while (S + _size >= draw_command.size())
				resize();

			std::memcpy(draw_command.ptrw() + _size, &d, S);
			_size += S;
		}

		void encode_color(const Color &color);
		void encode_draw_rect(float x, float y, float width, float height, const Color &c);
		void decode_draw_rect(GDMatPlotNative *self);
		void encode_draw_line(float x1, float y1, float x2, float y2, float width, const Color &c);
		void decode_draw_line(GDMatPlotNative *self);
		void encode_put_text(float x, float y, float text_angle, int font_size, int justify,
				const Color &c, const String &s);
		void decode_put_text(GDMatPlotNative *self);
		void encode_point(float x, float y, float size, const Color &c);
		void decode_point(GDMatPlotNative *self);
		void encode_filled_polygon(int point_count, const int *corners, const Color &c);
		void decode_filled_polygon(GDMatPlotNative *self);
		void encode_init(float x, float y, float width, float height, const Color &c);
		void decode_init(GDMatPlotNative *self);
		void encode_vector(float x1, float y1, float x2, float y2, float width, const Color &c);
		void decode_vector(GDMatPlotNative *self);
		void encode_fillbox(float x, float y, float width, float height, const Color &c);
		void decode_fillbox(GDMatPlotNative *self);
		void encode_path_is_open(PackedVector2Array &points, unsigned int is_open,
				float width, const Color &c);
		void decode_path_is_open(GDMatPlotNative *self);
		void decode(GDMatPlotNative *self);
	};

	BackendParams _bp;
	PackedVector2Array _path_points;
	std::vector<EncodedDrawing> _encoded_drawings;
	GDMatPlotGNUPlotRenderer *_gnuplot_render_thread_func{};
	Ref<Thread> _gnuplot_render_thread;
	Ref<Mutex> _get_set_stage_lock;
	Ref<GDMatPlotBackend> _library;
	int _stage{ UNDEFINED_STAGE };
	float _background_alpha{ 1.0 };
	bool _antialiased{ false };

	static void _bind_methods();

	void _set_stage(int p_stage);
	int _get_stage();
	void _draw_plot();
	void _draw_path_points(bool is_open);

public:
	GDMatPlotNative();
	virtual ~GDMatPlotNative();

	void _draw() override;

	void set_background_transparency(float p_transparency);
	float get_background_transparency() const;
	void set_antialiased(bool p_antialiased);
	bool get_antialiased() const;

	Variant load_gnuplot(String p_path = GDMATPLOT_DEFAULT_LIBGNUPLOT_PATH);
	Variant run_command(String p_cmd);
	Variant set_dataframe(PackedFloat64Array frame, int p_column_count);
	Variant load_dataframe();
	Variant start_renderer(Callable p_loop_func);
	void set_rendering_period(int64_t p_period);
	void stop_renderer();

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
