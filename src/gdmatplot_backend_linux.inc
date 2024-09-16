/**************************************************************************/
/*  gdmatplot_backend_linux.inc                                           */
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

#include <dlfcn.h>
#define GDMATPLOT_DLOPEN_MODE RTLD_NOW | RTLD_DEEPBIND

int GDMatPlotBackend::load(String &p_path) {
	int result = GDMatPlotBackendBase::load(p_path);

	_is_loaded = false;

	if (result) {
		set_status(result);
		return result;
	}

	unload();

	char *error = nullptr;

	_handle = dlopen(_path, GDMATPLOT_DLOPEN_MODE);
	if (!_handle) {
		GDMATPLOT_ERROR("%s\n", dlerror());

		set_status(ERR_READ_LIB);
		return ERR_READ_LIB;
	}

	dlerror();

	_gdmp_init = (gdmatplot_gdmp_init_t)dlsym(_handle, "gdmp_init");
	error = dlerror();
	if (error) {
		GDMATPLOT_ERROR("%s\n", error);

		result = ERR_SYM_NOT_FOUND;
		goto failed;
	}

	_set_gdmp_input_line = (gdmatplot_set_gdmp_input_line_t)dlsym(_handle, "set_gdmp_input_line");
	error = dlerror();
	if (error) {
		GDMATPLOT_ERROR("%s\n", error);

		result = ERR_SYM_NOT_FOUND;
		goto failed;
	}

	_do_line = (gdmatplot_do_line_t)dlsym(_handle, "do_line");
	error = dlerror();
	if (error) {
		GDMATPLOT_ERROR("%s\n", error);

		result = ERR_SYM_NOT_FOUND;
		goto failed;
	}

	_set_gdmp_matplot_object = (gdmatplot_set_gdmp_matplot_object_t)dlsym(_handle, "set_gdmp_matplot_object");
	error = dlerror();
	if (error) {
		GDMATPLOT_ERROR("%s\n", error);

		result = ERR_SYM_NOT_FOUND;
		goto failed;
	}

	_set_gdmp_dataframe = (gdmatplot_set_gdmp_dataframe_t)dlsym(_handle, "set_gdmp_dataframe");
	error = dlerror();
	if (error) {
		GDMATPLOT_ERROR("%s\n", error);

		result = ERR_SYM_NOT_FOUND;
		goto failed;
	}

	_gdmp_get_gnuplot_version = (gdmatplot_gdmp_get_gnuplot_version_t)dlsym(_handle, "gdmp_get_gnuplot_version");
	error = dlerror();
	if (error) {
		GDMATPLOT_ERROR("%s\n", error);

		result = ERR_SYM_NOT_FOUND;
		goto failed;
	}

	_is_loaded = true;

	set_status(0);
	return 0;

failed:
	unload();

	set_status(result);

	return result;
}

int GDMatPlotBackend::unload() {
	int result = 0;

	if (_handle != nullptr) {
#ifndef GDMATPLOT_MEMTEST_BUILD
		result = dlclose(_handle);
#endif
		_handle = nullptr;
		set_status(result);
	} else {
		set_status(ERR_NOT_INITIALIZED);
		result = ERR_NOT_INITIALIZED;
	}

	GDMatPlotBackendBase::unload();

	GDMATPLOT_DEBUG("gnuplot is unloaded with error code: %d", result);

	return result;
}

int GDMatPlotBackend::init(GDMatPlotNative *mp_ptr) {
	int error = GDMatPlotBackendBase::init(mp_ptr);
	if (error) {
		set_status(error);
		return error;
	}

	_set_gdmp_matplot_object(&_gnuplot_interface);
	_gdmp_init();

	set_status(0);
	return 0;
}

int GDMatPlotBackend::run_command(String &cmd) {
	const CharString &raw = cmd.ascii();
	_set_gdmp_input_line(raw, raw.size());

	return _do_line();
}

int GDMatPlotBackend::set_dataframe(PackedFloat64Array &frame, int column_count) {
	int error = GDMatPlotBackendBase::set_dataframe(frame, column_count);
	if (error) {
		set_status(error);
		return error;
	}

	int size = _dataframe.size();
	_set_gdmp_dataframe(_dataframe.ptr(), size / column_count, column_count);

	return 0;
}

int GDMatPlotBackend::load_dataframe() {
	int error = GDMatPlotBackendBase::load_dataframe();
	if (error) {
		set_status(error);
		return error;
	}

	int size = _dataframe.size();
	_set_gdmp_dataframe(_dataframe.ptr(), size / _dataframe_column_count, _dataframe_column_count);

	return 0;
}
