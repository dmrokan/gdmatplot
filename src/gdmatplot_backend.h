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

struct GDMatPlotGNUPlotInterface;

namespace godot {
class GDMatPlotNative;

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
	std::shared_ptr<::GDMatPlotGNUPlotInterface> _gnuplot_interface{};

	gdmatplot_gdmp_init_t _gdmp_init{};
	gdmatplot_set_gdmp_input_line_t _set_gdmp_input_line{};
	gdmatplot_do_line_t _do_line{};
	gdmatplot_set_gdmp_matplot_object_t _set_gdmp_matplot_object{};
	gdmatplot_set_gdmp_dataframe_t _set_gdmp_dataframe{};
	gdmatplot_gdmp_get_gnuplot_version_t _gdmp_get_gnuplot_version{};

	int _status{};
	int _dataframe_column_count{};
	bool _is_initialized{false};

	static void _bind_methods() {}

public:
	GDMatPlotBackendBase();
	virtual ~GDMatPlotBackendBase();

	void set_path(String &p_path) {
		_path = p_path.ascii();
	}

	virtual int load(String &p_path) {
		_is_initialized = false;

		if (!FileAccess::file_exists(p_path))
			return ERR_NOT_FOUND;

		unload();

		set_path(p_path);

		_is_initialized = true;

		return 0;
	};

	virtual int unload() {
		_is_initialized = false;

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

	bool is_initialized() {
		return _is_initialized;
	}
};

#if defined(LINUX_ENABLED)
class GDMatPlotBackendLinux : public GDMatPlotBackendBase {
	GDCLASS(GDMatPlotBackendLinux, GDMatPlotBackendBase)

protected:
	static void _bind_methods() {}

	gdmp_lib_handle_t _handle{};

public:
	~GDMatPlotBackendLinux() {
		unload();
	}

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
	~GDMatPlotBackendWindows() {
		unload();
	}

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
