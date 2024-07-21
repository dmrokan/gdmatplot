/**************************************************************************/
/*  gdmatplot_backend.cpp                                                 */
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

#include "gdmatplot_backend.h"

#include "gdmatplot.h"

#define SELF ((GDMatPlot *) self)

typedef godot::GDMatPlotNative GDMatPlot;

#include "gdmatplot_backend.inc"

using namespace godot;

GDMatPlotBackendBase::GDMatPlotBackendBase() : _gnuplot_interface(std::make_shared<::GDMatPlotGNUPlotInterface>()) {
}

int GDMatPlotBackendBase::init(GDMatPlotNative *mp_ptr) {
	_mp_ptr = mp_ptr;

	_gnuplot_interface->self = mp_ptr;

	return 0;
}

#if defined(LINUX_ENABLED)
#include "gdmatplot_backend_linux.inc"
#elif defined(WINDOWS_ENABLED)
#include "gdmatplot_backend_windows.inc"
#endif
