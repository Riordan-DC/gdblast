#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

BIN_NAME = "gdblast"

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
CPPPATHS = ["src/"]
LIBS = [""]
sources = Glob("src/*.cpp")

# GDBlast
sources += Glob("src/gdblast/*.cpp")
CPPPATHS += [
    "src/gdblast/nvblast/",
    "src/gdblast/nvblast/sdk/common",
    "src/gdblast/nvblast/sdk/lowlevel/include",
    "src/gdblast/nvblast/sdk/lowlevel/source",
    "src/gdblast/nvblast/sdk/toolkit/include",
    "src/gdblast/nvblast/sdk/toolkit/source",
    "src/gdblast/nvblast/sdk/globals/include",
    "src/gdblast/nvblast/sdk/globals/source",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/include",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/include/pvd",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/include/task",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/include/filebuf",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/include/foundation",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/src/foundation/include",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/src/filebuf/include",
    "src/PhysX-win.x86_64.vc150/3.4.24990349/PxShared/src/fastxml/include",
]
# Issue: these are built using a different compiler? or I just need to put the dlls in the file
LIBS += [
    "src/gdblast/nvblast/lib/NvBlastTkDEBUG_x64.lib",
    "src/gdblast/nvblast/lib/NvBlastDEBUG_x64.lib",
    "src/gdblast/nvblast/lib/NvBlastExtShadersDEBUG_x64.lib",
    "src/gdblast/nvblast/lib/NvBlastExtAssetUtilsDEBUG_x64.lib",
    "src/gdblast/nvblast/lib/NvBlastGlobalsDEBUG_x64.lib",
    "src/gdblast/nvblast/lib/NvBlastExtStressDEBUG_x64.lib"
]

env.Append(CPPPATH=CPPPATHS)
env.Append(LIBS=LIBS)

if env['platform'] == "macos":
    library = env.SharedLibrary(
        f"project/bin/{BIN_NAME}.{env['platform']}.{env['target']}.framework/{BIN_NAME}.{env['platform']}.{env['target']}",
        source=sources,
    )
elif env['platform'] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            f"project/bin/{BIN_NAME}.{env['platform']}.{env['target']}.simulator.a",
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            f"project/bin/{BIN_NAME}.{env['platform']}.{env['target']}.a",
            source=sources,
        )
else:
    library = env.SharedLibrary(
        f"project/bin/{BIN_NAME}{env['suffix']}{env['SHLIBSUFFIX']}",
        source=sources,
    )

Default(library)
