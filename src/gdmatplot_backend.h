/**************************************************************************/
/*  gdmatplot_backend.h                                                   */
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

#ifndef GDMATPLOT_BACKEND_H_
#define GDMATPLOT_BACKEND_H_

#include <memory>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/char_string.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "utils.h"

#define GDMATPLOT_GNUPLOT_VERSION_STR_MAXLEN (32)

#if defined(LINUX_ENABLED)
#define GDMP_STDCALL
typedef void *gdmp_lib_handle_t;
#elif defined(WINDOWS_ENABLED)
#include <windows.h>
#define GDMP_STDCALL __stdcall
typedef HINSTANCE gdmp_lib_handle_t;
#endif

typedef int(GDMP_STDCALL *gdmatplot_gdmp_init_t)(void);
typedef int(GDMP_STDCALL *gdmatplot_do_line_t)(void);
typedef void(GDMP_STDCALL *gdmatplot_set_gdmp_input_line_t)(const char *, size_t);
typedef int(GDMP_STDCALL *gdmatplot_set_gdmp_matplot_object_t)(void *);
typedef void(GDMP_STDCALL *gdmatplot_set_gdmp_dataframe_t)(const double *, unsigned int, unsigned int);
typedef int(GDMP_STDCALL *gdmatplot_gdmp_get_gnuplot_version_t)(char *, unsigned int);

namespace godot {
class GDMatPlotNative;

struct GDMatPlotMethods {
	void (*options)(void *);
	void (*init)(void *);
	void (*reset)(void *);
	void (*text)(void *);
	int (*scale)(void *, double, double);
	void (*graphics)(void *);
	void (*move)(void *, unsigned int, unsigned int);
	void (*vector)(void *, unsigned int, unsigned int);
	void (*linetype)(void *, int);
	void (*put_text)(void *, unsigned int, unsigned int, const char *);
	int (*text_angle)(void *, float); /* Changed from int to double in version 5.5 */
	int (*justify_text)(void *, int);
	void (*point)(void *, unsigned int, unsigned int, int);
	void (*arrow)(void *, unsigned int, unsigned int, unsigned int, unsigned int, int headstyle);
	int (*set_font)(void *, const char *, double);
	void (*pointsize)(void *, double); /* change pointsize */
	void (*suspend)(void *); /* called after one plot of multiplot */
	void (*resume)(void *); /* called before plots of multiplot */
	void (*fillbox)(void *, int, unsigned int, unsigned int, unsigned int, unsigned int); /* clear in multiplot mode */
	void (*linewidth)(void *, double linewidth);

	int (*make_palette)(void *, void *palette);
	void (*previous_palette)(void *);
	void (*set_color)(void *, unsigned int);
	void (*filled_polygon)(void *, int, void *, int);
	void (*image)(void *, unsigned int, unsigned int, void *, void *, unsigned int);
	void (*enhanced_open)(void *, char *fontname, double fontsize,
			double base, char widthflag, char showflag,
			int overprint);
	void (*enhanced_flush)(void *);
	void (*enhanced_writec)(void *, int c);
	void (*layer)(void *, unsigned int);
	void (*path)(void *, int p);
	void (*hypertext)(void *, int type, const char *text);
	void (*boxed_text)(void *, unsigned int, unsigned int, int);
	void (*modify_plots)(void *, unsigned int operations, int plotno);
	void (*dashtype)(void *, int type, void *custom_dash_pattern);

	/* ======================================== */

	void (*set_xmax)(void *, unsigned int);
	void (*set_ymax)(void *, unsigned int);
	void (*set_h_tic)(void *, unsigned int);
	void (*set_v_tic)(void *, unsigned int);

	void (*set_color_mode)(void *, unsigned char);
	void (*set_linetype)(void *, int);
	void (*set_dashpattern)(void *, const char *);

	void (*set_h_char)(void *, unsigned int);
	void (*set_v_char)(void *, unsigned int);

	void (*set_gridline)(void *, unsigned int);
	void (*set_hasgrid)(void *, unsigned int);
	void (*set_plotno)(void *, unsigned int);

	void (*set_fill_pattern)(void *, int);
	void (*set_fill_pattern_index)(void *, unsigned int);
	void (*set_rgb)(void *, unsigned int);
	void (*set_patterncolor)(void *, unsigned int *);
	void (*set_group_filled_is_open)(void *, unsigned char);
	void (*set_in_textbox)(void *, unsigned char);
	void (*set_xsize)(void *, unsigned int);
	void (*set_ysize)(void *, unsigned int);
	void (*set_xlast)(void *, unsigned int);
	void (*set_ylast)(void *, unsigned int);
	void (*set_linecap)(void *, int);
	void (*set_group_is_open)(void *, unsigned char);
	void (*set_path_is_open)(void *, unsigned char);

	void (*set_fontscale)(void *, double);
	void (*set_dashlength)(void *, double);
	void (*set_name)(void *, const char *);
	void (*set_linewidth_factor)(void *, double);
	void (*set_background)(void *, int);

	void (*set_linecolor)(void *, const char *);
	void (*set_alpha)(void *, double);

	void (*set_term_pointsize)(void *, double);
	void (*set_stroke_width)(void *, double);

	void (*set_pen)(void *, unsigned int, unsigned int, double);

	/* ======================================== */

	double tscale;
	int flags;

	GDMatPlotMethods();
};

struct GDMatPlotGNUPlotInterface {
	GDMatPlotMethods methods;
	GDMatPlotNative *self;
};

class GDMatPlotBackendBase : public RefCounted {
	GDCLASS(GDMatPlotBackendBase, RefCounted)

protected:
	enum {
		ERR_GENERAL = std::numeric_limits<short>::lowest(),
		ERR_NOT_INITIALIZED,
		ERR_READ_LIB,
		ERR_NOT_FOUND,
		ERR_SYM_NOT_FOUND,
		ERR_INVALID_DIM,
	};

	PackedFloat64Array _dataframe;
	CharString _path;
	GDMatPlotNative *_mp_ptr{};
	GDMatPlotGNUPlotInterface _gnuplot_interface;

	gdmatplot_gdmp_init_t _gdmp_init{};
	gdmatplot_set_gdmp_input_line_t _set_gdmp_input_line{};
	gdmatplot_do_line_t _do_line{};
	gdmatplot_set_gdmp_matplot_object_t _set_gdmp_matplot_object{};
	gdmatplot_set_gdmp_dataframe_t _set_gdmp_dataframe{};
	gdmatplot_gdmp_get_gnuplot_version_t _gdmp_get_gnuplot_version{};

	int _status{};
	int _dataframe_column_count{};
	bool _is_loaded{ false };

	static void _bind_methods() {}

public:
	GDMatPlotBackendBase();

	void set_path(String &p_path) {
		_path = p_path.ascii();
	}

	virtual int load(String &p_path) {
		_is_loaded = false;

		if (!FileAccess::file_exists(p_path))
			return ERR_NOT_FOUND;

		unload();

		set_path(p_path);

		_is_loaded = true;

		return 0;
	};

	virtual int unload() {
		_is_loaded = false;

		return ERR_NOT_INITIALIZED;
	};

	virtual int init(GDMatPlotNative *mp_ptr);

	virtual int get_status() const {
		return _status;
	}

	virtual void set_status(int p_status) {
		_status = p_status;
	}

	virtual int run_command(String &cmd) {
		return ERR_NOT_INITIALIZED;
	}

	virtual int set_dataframe(PackedFloat64Array &frame, int column_count) {
		_dataframe = frame;
		_dataframe_column_count = column_count;

		return 0;
	}

	virtual int load_dataframe() {
		if (_dataframe.size() > 0)
			return 0;

		return ERR_INVALID_DIM;
	}

	String get_gnuplot_version() {
		String vstr;
		char cstr[GDMATPLOT_GNUPLOT_VERSION_STR_MAXLEN];

		_gdmp_get_gnuplot_version(cstr, GDMATPLOT_GNUPLOT_VERSION_STR_MAXLEN);

		return String(cstr);
	}

	bool is_loaded() {
		return _is_loaded;
	}
};

#if defined(LINUX_ENABLED)
class GDMatPlotBackendLinux : public GDMatPlotBackendBase {
	GDCLASS(GDMatPlotBackendLinux, GDMatPlotBackendBase)

protected:
	static void _bind_methods() {}

	gdmp_lib_handle_t _handle{};

public:
	int load(String &p_path) override;
	int unload() override;
	int init(GDMatPlotNative *mp_ptr) override;
	int run_command(String &cmd) override;
	int set_dataframe(PackedFloat64Array &frame, int column_count) override;
	int load_dataframe() override;
};

typedef GDMatPlotBackendLinux GDMatPlotBackend;
#elif defined(WINDOWS_ENABLED)
class GDMatPlotBackendWindows : public GDMatPlotBackendBase {
	GDCLASS(GDMatPlotBackendWindows, GDMatPlotBackendBase)

protected:
	static void _bind_methods() {}

	gdmp_lib_handle_t _handle{};

public:
	int load(String &p_path) override;
	int unload() override;
	int init(GDMatPlotNative *mp_ptr) override;
	int run_command(String &cmd) override;
	int set_dataframe(PackedFloat64Array &frame, int column_count) override;
	int load_dataframe() override;
};

typedef GDMatPlotBackendWindows GDMatPlotBackend;
#else

typedef GDMatPlotBackendBase GDMatPlotBackend;
#endif
} //namespace godot

#endif // GDMATPLOT_BACKEND_H_
