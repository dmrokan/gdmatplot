from subprocess import run, CalledProcessError
import os
import time

try:
	run(["rm", "-f", "gdmp.trm"], cwd="./gnuplot/term", check=True)
	run(["rm", "-f", "gdmp_disable_io.h"], cwd="./gnuplot/src", check=True)
	run(["git", "restore", "."], cwd="./gnuplot", check=True)
	run(["git", "apply", "../gdmatplot-gnuplot.patch"], cwd="./gnuplot", check=True)
	run(["rm", "-f", "libgnuplot.so"], check=True)
	run(["rm", "-f", "libgnuplot.dll"], check=True)
	run(["rm", "-f", "src/gdmatplot_gnuplot_library.cxx"], check=True)
except CalledProcessError as e:
	print(e)
	Exit(2)

SOURCE = [Glob('gnuplot/src/*.c', exclude=[
	'gnuplot/src/gplt_x11.c', 'gnuplot/src/vms.c', 'gnuplot/src/bf_test.c',
])]

SOURCE = [Glob('gnuplot/src/*.c', exclude=[
	'gnuplot/src/gplt_x11.c', 'gnuplot/src/vms.c', 'gnuplot/src/bf_test.c',
	'gnuplot/src/xdg.c', 'gnuplot/src/bitmap.c', 'gnuplot/src/breaders.c',
	'gnuplot/src/external.c', 'gnuplot/src/gpexecute.c', 'gnuplot/src/help.c',
	'gnuplot/src/history.c', 'gnuplot/src/mouse.c', 'gnuplot/src/save.c',

	'gnuplot/src/os2/print.c', 'gnuplot/src/os2/dialogs.c', 'gnuplot/src/os2/gclient.c',
	'gnuplot/src/os2/gnupmdrv.c',

	'gnuplot/src/beos/GPApp.c', 'gnuplot/src/beos/GPView.c', 'gnuplot/src/beos/GPBitmap.c',
	'gnuplot/src/beos/GPWindow.c', 'gnuplot/src/beos/XStringList.cpp',

	'gnuplot/src/win/pgnuplot.c', 'gnuplot/src/win/screenbuf.c', 'gnuplot/src/win/wgnuplib.c',
	'gnuplot/src/win/wgraph.c',	'gnuplot/src/win/winmain.c', 'gnuplot/src/win/wmenu.c',
	'gnuplot/src/win/wpause.c', 'gnuplot/src/win/wprinter.c', 'gnuplot/src/win/wtext.c',
])]

SOURCE += [Glob('gnuplot/term/*.c', exclude=[
	'gnuplot/term/sixel.c', 'gnuplot/term/kitty.c', 'gnuplot/term/write_png_image.c',
])]

DFLAGS = [
	'HAVE_STRING_H', 'HAVE_STDLIB_H', 'HAVE_MALLOC_H',
	'HAVE_MEMCPY', 'HAVE_STRCHR', 'HAVE_VFPRINTF',
	'STDC_HEADERS', 'HAVE_ERRNO_H', 'HAVE_LIMITS_H',
	'HAVE_TIME_H', 'HAVE_TIME_T_IN_TIME_H', 'HAVE_FLOAT_H',
	'HAVE_MATH_H', 'HAVE_STRNDUP', 'HAVE_STRNLEN',
	'HAVE__BOOL', 'HAVE_INTTYPES_H', 'HAVE_ERF',
	'HAVE_ERFC', 'USE_WATCHPOINTS',	'HAVE_OFF_T',
	'NO_BITMAP_SUPPORT', 'NO_GIH',

	'HAVE_STRCASECMP', 'HAVE_STRNCASECMP', 'HAVE_ERRNO_H',
	'HAVE_STRERROR',

	'WINDOWS_NO_GUI',

	'GDMP_BUILD', 'GDMP_DISABLE_IO',
]

platform = ARGUMENTS.get('platform', "linux")
target = ARGUMENTS.get('target', "template_release")

CPPFLAGS = []

if target == "template_release":
	CPPFLAGS += ['-O2']
elif target == "template_debug":
	CPPFLAGS += ['-O0', '-g']

for f in DFLAGS:
	CPPFLAGS.append('-D' + f)

TOOL = 'gcc'
if platform == "windows":
	TOOL = 'mingw'
	idx = CPPFLAGS.index('-DHAVE_STRNDUP')
	del CPPFLAGS[idx]

lib_env = Environment(CPPPATH=['gnuplot/src', 'gnuplot/term'], CPPFLAGS=CPPFLAGS, LIBS=['m'])
lib_env.Append(tools=[TOOL])

if platform == "windows":
	lib_env['CC'] = 'x86_64-w64-mingw32-gcc'

dump_filename = "src/gdmatplot_gnuplot_library.cxx"

libraryfile = "libgnuplot."
if platform == "windows":
	libraryfile += "dll"
elif platform == "linux":
	libraryfile += "so"

lib = lib_env.SharedLibrary(libraryfile, source=SOURCE)

def lib_dump(target, source, env):
	with open(dump_filename, "w") as fo:
		fo.write("static const unsigned char gnuplot_library[] = {\n")
		with open(libraryfile, "rb") as fi:
			b = fi.read(1)
			i = 0
			while len(b) > 0:
				fo.write("0x" + b.hex() + ", ")
				i += 1
				b = fi.read(1)

				if i % 16 == 0:
					fo.write("\n")

		fo.write("\n};\n"); print("Written {} bytes".format(i))
		fo.write("static const unsigned int gnuplot_library_size = %u;\n" % i)
		fo.write("const unsigned char *gdmatplot_gnuplot_library_data() { return gnuplot_library; }\n")
		fo.write("const unsigned int gdmatplot_gnuplot_library_size() { return gnuplot_library_size; }\n")

		fo.flush()

	os.sync()

lib_dump_command = Command('lib_dump', [], lib_dump)
AlwaysBuild(lib_dump_command)
Requires(lib_dump_command, lib)

