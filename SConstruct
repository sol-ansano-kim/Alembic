import os
import sys
import re
import ast
import glob
import excons
from excons.tools import python
from excons.tools import arnold
from excons.tools import maya
from excons.tools import houdini
from excons.tools import boost
from excons.tools import gl
from excons.tools import glut
from excons.tools import glew


defs    = []
gldefs  = ["GLEW_STATIC", "GLEW_NO_GLU"] if sys.platform != "darwin" else []
glreqs  = [glew.Require] if sys.platform != "darwin" else [gl.Require]
pydefs  = ["BOOST_PYTHON_DYNAMIC_LIB", "BOOST_PYTHON_NO_LIB"]
incdirs = ["lib"]
libdirs = []
libs    = []

if sys.platform == "win32":
   #defs.extend(["PLATFORM_WINDOWS", "PLATFORM=WINDOWS"])
   defs.extend(["PLATFORM_WINDOWS"])
elif sys.platform == "darwin":
   defs.extend(["PLATFORM_DARWIN", "PLATFORM=DARWIN"])
   libs.append("m")
else:
   defs.extend(["PLATFORM_LINUX", "PLATFORM=LINUX"])
   libs.extend(["dl", "rt", "m"])

hdf5_libs = []

ilmbase_libs = ["IlmThread", "Imath", "IexMath", "Iex", "Half"]

ilmbasepy_libs = ["PyImath"]

alembic_libs = ["AlembicAbcMaterial",
                "AlembicAbcGeom",
                "AlembicAbcCollection",
                "AlembicAbcCoreFactory",
                "AlembicAbc",
                "AlembicAbcCoreHDF5",
                "AlembicAbcCoreOgawa",
                "AlembicAbcCoreAbstract",
                "AlembicOgawa",
                "AlembicUtil"]

alembicgl_libs = ["AlembicAbcOpenGL"]

regex_def = []
regex_inc = []
regex_src = []



def AddDirectories(inc_dir, lib_dir):
   global incdirs, libdirs
   
   if inc_dir and os.path.isdir(inc_dir) and not inc_dir in incdirs:
      incdirs.append(inc_dir)
   
   if lib_dir and os.path.isdir(lib_dir) and not lib_dir in libdirs:
      libdirs.append(lib_dir)

def SafeRemove(lst, item):
   try:
      lst.remove(item)
   except:
      pass
   return lst


# Default dependencies include and library path
deps_inc, deps_lib = excons.GetDirs("deps", noexc=True, silent=True)
AddDirectories(deps_inc, deps_lib)

# Boost library setup
boost_inc, boost_lib = excons.GetDirsWithDefault("boost", incdirdef=deps_inc, libdirdef=deps_lib)
AddDirectories(boost_inc, boost_lib)

# Boost python setup
boostpy_inc, boostpy_lib = excons.GetDirsWithDefault("boost-python", incdirdef=deps_inc, libdirdef=deps_lib)
boostpy_libname = excons.GetArgument("boost-python-libname", "boost_python")
AddDirectories(boostpy_inc, boostpy_lib)

# HDF5 library setup
hdf5_threadsafe = False
hdf5_zlib = False
hdf5_szip = False

hdf5_inc, hdf5_lib = excons.GetDirsWithDefault("hdf5", incdirdef=deps_inc, libdirdef=deps_lib)
AddDirectories(hdf5_inc, hdf5_lib)
hdf5_libname = excons.GetArgument("hdf5-libname", ("hdf5" if sys.platform != "win32" else "libhdf5"))
hdf5_libs.extend([hdf5_libname+"_hl", hdf5_libname])

h5conf = os.path.join(hdf5_inc, "H5pubconf.h")
if os.path.isfile(h5conf):
  f = open(h5conf, "r")
  tse = re.compile(r"^\s*#define\s+H5_HAVE_THREADSAFE\s+1")
  sze = re.compile(r"^\s*#define\s+H5_HAVE_SZLIB_H\s+1")
  zle = re.compile(r"^\s*#define\s+H5_HAVE_ZLIB_H\s+1")
  for l in f.readlines():
    l = l.strip()
    if tse.match(l):
      print("HDF5 thread safe")
      hdf5_threadsafe = True
    elif sze.match(l):
      print("HDF5 using szip")
      hdf5_szip = True
    elif zle.match(l):
      print("HDF5 using zlib")
      hdf5_zlib = True
  f.close()

if hdf5_threadsafe and sys.platform != "win32":
  hdf5_libs.append("pthread")

if hdf5_zlib:
  zlib_inc, zlib_lib = excons.GetDirsWithDefault("zlib", incdirdef=deps_inc, libdirdef=deps_lib)
  AddDirectories(zlib_inc, zlib_lib)
  zlib_libname = excons.GetArgument("zlib-libname", ("z" if sys.platform != "win32" else "zlib"))
  hdf5_libs.append(zlib_libname)

if hdf5_szip:
  szip_inc, szip_lib = excons.GetDirsWithDefault("szip", incdirdef=deps_inc, libdirdef=deps_lib)
  AddDirectories(szip_inc, szip_lib)
  szip_libname = excons.GetArgument("szip-libname", ("sz" if sys.platform != "win32" else "libszip"))
  hdf5_libs.append(szip_libname)

# IlmBase library setup
ilmbase_inc, ilmbase_lib = excons.GetDirsWithDefault("ilmbase", incdirdef=deps_inc, libdirdef=deps_lib)
ilmbase_libsuffix = excons.GetArgument("ilmbase-libsuffix", "")
if ilmbase_inc and not ilmbase_inc.endswith("OpenEXR"):
  ilmbase_inc += "/OpenEXR"
if ilmbase_libsuffix:
  ilmbase_libs = map(lambda x: x+ilmbase_libsuffix, ilmbase_libs)
AddDirectories(ilmbase_inc, ilmbase_lib)

# IlmBase python setup
ilmbasepy_inc, ilmbasepy_lib = excons.GetDirsWithDefault("ilmbase-python", incdirdef=deps_inc, libdirdef=deps_lib)
ilmbasepy_libsuffix = excons.GetArgument("ilmbase-python-libsuffix", ilmbase_libsuffix)
if ilmbasepy_inc and not ilmbasepy_inc.endswith("OpenEXR"):
  ilmbasepy_inc += "/OpenEXR"
if ilmbasepy_libsuffix:
  ilmbasepy_libs = map(lambda x: x+ilmbasepy_libsuffix, ilmbasepy_libs)
AddDirectories(ilmbasepy_inc, ilmbasepy_lib)

# Others
nameprefix = excons.GetArgument("name-prefix", default="")

# arnold, maya, python, glut, glew: using excons tools
# (GLEW default setup: glew-static=1, glew-no-glu=1, glew-mx=0)

env = excons.MakeBaseEnv()

if sys.platform == "darwin":
   import platform
   vers = map(int, platform.mac_ver()[0].split("."))
   if vers[0] > 10 or vers[1] >= 9:
      # Shut clang's up
      env.Append(CPPFLAGS=" ".join([" -std=c++11",
                                    "-Wno-deprecated-register",
                                    "-Wno-deprecated-declarations",
                                    "-Wno-missing-field-initializers",
                                    "-Wno-unused-parameter",
                                    "-Wno-unused-value",
                                    "-Wno-unused-function",
                                    "-Wno-unused-variable",
                                    "-Wno-unused-private-field"]))
elif sys.platform == "win32":
   env.Append(CCFLAGS = " /bigobj")
   defs.extend(["NOMINMAX", "OPENEXR_DLL"])
   regex_dir = "lib/SceneHelper/regex-2.7/src"
   regex_def = ["REGEX_STATIC"]
   regex_inc = [regex_dir]
   regex_src = [regex_dir + "/regex.c"]

prjs = [
   {"name": "AlembicUtil",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/Util/*.cpp"),
    "install": {"include/Alembic/Util": glob.glob("lib/Alembic/Util/*.h")}
   },
   {"name": "AlembicAbcCoreAbstract",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/AbcCoreAbstract/*.cpp"),
    "install": {"include/Alembic/AbcCoreAbstract": glob.glob("lib/Alembic/AbcCoreAbstract/*.h")}
   },
   {"name": "AlembicAbcCoreOgawa",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/AbcCoreOgawa/*.cpp"),
    "install": {"include/Alembic/AbcCoreOgawa": ["lib/Alembic/AbcCoreOgawa/All.h",
                                                 "lib/Alembic/AbcCoreOgawa/ReadWrite.h"]}
   },
   {"name": "AlembicAbcCoreHDF5",
   "defs": defs,
    "type": "staticlib",
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/AbcCoreHDF5/*.cpp"),
    "install": {"include/Alembic/AbcCoreHDF5": ["lib/Alembic/AbcCoreHDF5/All.h",
                                                "lib/Alembic/AbcCoreHDF5/ReadWrite.h"]}
   },
   {"name": "AlembicAbc",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/Abc/*.cpp"),
    "install": {"include/Alembic/Abc": glob.glob("lib/Alembic/Abc/*.h")}
   },
   {"name": "AlembicAbcCoreFactory",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/AbcCoreFactory/*.cpp"),
    "install": {"include/Alembic/AbcCoreFactory": glob.glob("lib/Alembic/AbcCoreFactory/*.h")}
   },
   {"name": "AlembicAbcGeom",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/AbcGeom/*.cpp"),
    "install": {"include/Alembic/AbcGeom": glob.glob("lib/Alembic/AbcGeom/*.h")}
   },
   {"name": "AlembicAbcCollection",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/AbcCollection/*.cpp"),
    "install": {"include/Alembic/AbcCollection": glob.glob("lib/Alembic/AbcCollection/*.h")}
   },
   {"name": "AlembicAbcMaterial",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/AbcMaterial/*.cpp"),
    "install": {"include/Alembic/AbcMaterial": SafeRemove(glob.glob("lib/Alembic/AbcMaterial/*.h"), ("lib/Alembic/AbcMaterial/InternalUtil.h"))}
   },
   {"name": "AlembicOgawa",
    "type": "staticlib",
    "defs": defs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/Alembic/Ogawa/*.cpp"),
    "install": {"include/Alembic/Ogawa": glob.glob("lib/Alembic/Ogawa/*.h")}
   },
   {"name": ("alembicmodule" if sys.platform != "win32" else "alembic"),
    "alias": "pyalembic",
    "type": "dynamicmodule",
    "ext": python.ModuleExtension(),
    "prefix": "%s/%s" % (python.ModulePrefix(), python.Version()),
    "defs": defs + pydefs + ["alembicmodule_EXPORTS"],
    "incdirs": incdirs + ["python/PyAlembic"],
    "libdirs": libdirs,
    "libs": alembic_libs + [boostpy_libname] + ilmbasepy_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("python/PyAlembic/*.cpp"),
    "custom": [python.SoftRequire],
    "install": {"%s/%s" % (python.ModulePrefix(), python.Version()): "python/examples/cask/cask.py"}
   },
   {"name": "abcconvert",
    "type": "program",
    "defs": defs,
    "incdirs": incdirs,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/AbcConvert/*.cpp")
   },
   {"name": "abcecho",
    "type": "program",
    "defs": defs,
    "incdirs": incdirs,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/AbcEcho/AbcEcho.cpp")
   },
   {"name": "abcboundsecho",
    "type": "program",
    "defs": defs,
    "incdirs": incdirs,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/AbcEcho/AbcBoundsEcho.cpp")
   },
   {"name": "abcls",
    "type": "program",
    "defs": defs,
    "incdirs": incdirs,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/AbcLs/*.cpp")
   },
   {"name": "abcstitcher",
    "type": "program",
    "defs": defs,
    "incdirs": incdirs,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/AbcStitcher/*.cpp")
   },
   {"name": "abctree",
    "type": "program",
    "defs": defs,
    "incdirs": incdirs,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/AbcTree/*.cpp")
   },
   {"name": "abcwalk",
    "type": "program",
    "defs": defs,
    "incdirs": incdirs,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/AbcWalk/*.cpp")
   },
   # OpenGL targets
   {"name": "AlembicAbcOpenGL",
    "type": "staticlib",
    "defs": defs + gldefs,
    "incdirs": incdirs,
    "srcs": glob.glob("lib/AbcOpenGL/*.cpp"),
    "custom": [glut.Require],
    "install": {"include/AbcOpenGL": glob.glob("lib/AbcOpenGL/*.h")}
   },
   {"name": ("alembicglmodule" if sys.platform != "win32" else "alembicgl"),
    "alias": "pyalembicgl",
    "type": "dynamicmodule",
    "ext": python.ModuleExtension(),
    "prefix": "%s/%s" % (python.ModulePrefix(), python.Version()),
    "defs": defs + pydefs + gldefs + ["alembicglmodule_EXPORTS"],
    "incdirs": incdirs + ["python/PyAbcOpenGL"],
    "libdirs": libdirs,
    "srcs": glob.glob("python/PyAbcOpenGL/*.cpp"),
    "libs": alembicgl_libs + alembic_libs + [boostpy_libname] + ilmbasepy_libs + ilmbase_libs + hdf5_libs + libs,
    "custom": glreqs + [glut.Require, python.SoftRequire]
   },
   {"name": "SimpleAbcViewer",
    "type": "program",
    "defs": defs + gldefs,
    "incdirs": incdirs + ["examples/bin/SimpleAbcViewer"],
    "libdirs": libdirs,
    "libs": alembicgl_libs + alembic_libs + ilmbase_libs + hdf5_libs + libs,
    "srcs": glob.glob("examples/bin/SimpleAbcViewer/*.cpp"),
    "custom": glreqs + [glut.Require]
   },
   {"name": "SceneHelper",
    "type": "program",
    "defs": defs + regex_def,
    "incdirs": incdirs + ["examples/bin/SceneHelper", "lib/SceneHelper"] + regex_inc,
    "libdirs": libdirs,
    "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs + (["pthread"] if sys.platform != "win32" else []),
    "srcs": glob.glob("examples/bin/SceneHelper/*.cpp") + glob.glob("lib/SceneHelper/*.cpp") + regex_src
   }
]

plugin_defs = defs[:]
if nameprefix:
   plugin_defs.append("NAME_PREFIX=\"\\\"%s\\\"\"" % nameprefix)

if excons.GetArgument("with-arnold", default=None) is not None:
   prjs.append({"name": "%sAlembicArnoldProcedural" % nameprefix,
                "type": "dynamicmodule",
                "ext": arnold.PluginExt(),
                "prefix": "arnold",
                "defs": plugin_defs,
                "incdirs": incdirs + ["arnold/Procedural"],
                "libdirs": libdirs,
                "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
                "srcs": glob.glob("arnold/Procedural/*.cpp"),
                "custom": [arnold.Require]})
   prjs.append({"name": "abcproc",
                "type": "dynamicmodule",
                "ext": arnold.PluginExt(),
                "prefix": "arnold",
                "defs": defs + regex_def,
                "incdirs": incdirs + ["arnold/abcproc"] + ["lib/SceneHelper"] + regex_inc,
                "libdirs": libdirs,
                "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
                "srcs": glob.glob("arnold/abcproc/*.cpp") + glob.glob("lib/SceneHelper/*.cpp") + regex_src,
                "custom": [arnold.Require]})

if excons.GetArgument("with-maya", default=None) is not None:
   def replace_in_file(src, dst, srcStr, dstStr):
      fdst = open(dst, "w")
      fsrc = open(src, "r")
      for line in fsrc.readlines():
         fdst.write(line.replace(srcStr, dstStr))
      fdst.close()
      fsrc.close()
   
   AbcShapeName = "%sAbcShape" % nameprefix
   AbcShapeMel = "maya/AbcShape/AE%sTemplate.mel" % AbcShapeName
   AbcShapeMtoa = "arnold/abcproc/mtoa_%s.py" % AbcShapeName
   
   if not os.path.exists(AbcShapeMel):
      replace_in_file("maya/AbcShape/AETemplate.mel.tpl", AbcShapeMel, "<<NodeName>>", AbcShapeName)
   if not os.path.exists(AbcShapeMtoa):
      replace_in_file("arnold/abcproc/mtoa.py.tpl", AbcShapeMtoa, "<<NodeName>>", AbcShapeName)
   
   prjs.extend([{"name": "%sAbcImport" % nameprefix,
                 "type": "dynamicmodule",
                 "ext": maya.PluginExt(),
                 "prefix": "maya/plug-ins",
                 "defs": plugin_defs,
                 "incdirs": incdirs + ["maya/AbcImport"],
                 "libdirs": libdirs,
                 "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
                 "srcs": glob.glob("maya/AbcImport/*.cpp"),
                 "install": {"maya/scripts": glob.glob("maya/AbcImport/*.mel")},
                 "custom": [maya.Require, maya.Plugin]
                },
                {"name": "%sAbcExport" % nameprefix,
                 "type": "dynamicmodule",
                 "ext": maya.PluginExt(),
                 "prefix": "maya/plug-ins",
                 "defs": plugin_defs,
                 "incdirs": incdirs + ["maya/AbcExport"],
                 "libdirs": libdirs,
                 "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
                 "srcs": glob.glob("maya/AbcExport/*.cpp"),
                 "install": {"maya/scripts": glob.glob("maya/AbcExport/*.mel")},
                 "custom": [maya.Require, maya.Plugin]
                },
                {"name": "%sAbcShape" % nameprefix,
                 "type": "dynamicmodule",
                 "ext": maya.PluginExt(),
                 "prefix": "maya/plug-ins",
                 "defs": plugin_defs + regex_def,
                 "incdirs": incdirs + ["maya/AbcShape", "lib/SceneHelper"] + regex_inc,
                 "libdirs": libdirs,
                 "libs": alembic_libs + ilmbase_libs + hdf5_libs + libs,
                 "srcs": glob.glob("maya/AbcShape/*.cpp") + glob.glob("lib/SceneHelper/*.cpp") + regex_src,
                 "install": {"maya/scripts": glob.glob("maya/AbcShape/*.mel"),
                             "maya/python": [AbcShapeMtoa]},
                 "custom": [maya.Require, maya.Plugin]}])

excons.DeclareTargets(env, prjs)
