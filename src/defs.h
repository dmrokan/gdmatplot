/**************************************************************************/
/*  defs.h                                                                */
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

#ifndef GDMATPLOT_DEFS_H_
#define GDMATPLOT_DEFS_H_

#include <cstdint>
#include <cstdio>

#define GDMATPLOT_SET_VERSION(a, b, c) (a << 20 | b << 10 | c)

static constexpr uint32_t GDMATPLOT_VERSION = GDMATPLOT_SET_VERSION(0, 2, 0);

#ifdef GDMATPLOT_DEBUG_PRINT

#define GDMATPLOT_ERROR(...)                                  \
	do {                                                      \
		gdmatplot_error(__VA_ARGS__, nullptr);                \
		fprintf(stderr, ": ( %s:%d )\n", __FILE__, __LINE__); \
	} while (0);

#define GDMATPLOT_WARN(...)                                   \
	do {                                                      \
		gdmatplot_warn(__VA_ARGS__, nullptr);                 \
		fprintf(stderr, ": ( %s:%d )\n", __FILE__, __LINE__); \
	} while (0);

#define GDMATPLOT_DEBUG(...)                                  \
	do {                                                      \
		gdmatplot_debug(__VA_ARGS__, nullptr);                \
		fprintf(stdout, ": ( %s:%d )\n", __FILE__, __LINE__); \
	} while (0);

#define GDMATPLOT_V_DEBUG(...)                                \
	do {                                                      \
		gdmatplot_debug(__VA_ARGS__, nullptr);                \
		fprintf(stdout, ": ( %s:%d )\n", __FILE__, __LINE__); \
	} while (0);

#else // GDMATPLOT_DEBUG_PRINT

#define GDMATPLOT_ERROR(...)                   \
	do {                                       \
		gdmatplot_error(__VA_ARGS__, nullptr); \
	} while (0);

#define GDMATPLOT_WARN(...) \
	do {                    \
	} while (0);

#define GDMATPLOT_DEBUG(...) \
	do {                     \
	} while (0);

#define GDMATPLOT_V_DEBUG(...) \
	do {                       \
	} while (0);

#endif

#endif // GDMATPLOT_DEF_H_
