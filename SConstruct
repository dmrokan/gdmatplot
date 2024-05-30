#!/usr/bin/env python
import os
import fnmatch


def normalize_path(val, env):
    return val if os.path.isabs(val) else os.path.join(env.Dir("#").abspath, val)


def validate_parent_dir(key, val, env):
    if not os.path.isdir(normalize_path(os.path.dirname(val), env)):
        raise UserError("'%s' is not a directory: %s" % (key, os.path.dirname(val)))


libname = "gdmatplot"
projectdir = "demo"

localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Add(
    BoolVariable(
        key="compiledb",
        help="Generate compilation DB (`compile_commands.json`) for external tools",
        default=localEnv.get("compiledb", False),
    )
)
opts.Add(
    PathVariable(
        key="compiledb_file",
        help="Path to a custom `compile_commands.json` file",
        default=localEnv.get("compiledb_file", "compile_commands.json"),
        validator=validate_parent_dir,
    )
)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()
env["compiledb"] = False

env.Tool("compilation_db")
compilation_db = env.CompilationDatabase(
    normalize_path(localEnv["compiledb_file"], localEnv)
)
env.Alias("compiledb", compilation_db)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

LIBS = []
if env["platform"] == "linux":
    LIBS = ['dl']

env.Append(CPPPATH=["src/"])
env.Append(CPPFLAGS=[])
env.Append(LIBS=LIBS)
sources = Glob("src/*.cpp") + ["src/gdmatplot_gnuplot_library.cxx"]

file = "{}{}{}".format(libname, env["suffix"], env["SHLIBSUFFIX"])

if env["platform"] == "macos":
    platlibname = "{}.{}.{}".format(libname, env["platform"], env["target"])
    file = "{}.framework/{}".format(env["platform"], platlibname, platlibname)

libraryfile = "demo/addons/gdmatplot/{}/{}".format(env["platform"], file)
library = env.SharedLibrary(
    libraryfile,
    source=sources,
)

copy_commands = []
for fn in [ "README.md", "LICENSE.md", "NOTICE.md" ]:
    copy_commands += [Command(f"demo/addons/gdmatplot/{fn}", fn, Copy("$TARGET", "$SOURCE"))]
    if len(copy_commands) > 1:
        Requires(copy_commands[-1], copy_commands[-2])
clone_addon = env.Install("demo3d/addons", "demo/addons/gdmatplot")
Requires(clone_addon, copy_commands[-1])
AlwaysBuild(clone_addon)

default_args = [library] + copy_commands + [clone_addon]
if localEnv.get("compiledb", False):
    default_args += [compilation_db]
Default(*default_args)
