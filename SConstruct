import excons
from excons.tools import nuke, freeimage
import glob

env = excons.MakeBaseEnv()

prjs = [
  {"name": "jp2Reader",
   "type": "dynamicmodule",
   "ext" : nuke.PluginExt(),
   "srcs": glob.glob("src/jp2Reader.cpp"),
   "custom": [freeimage.Require, nuke.Require]
  },
  {"name": "j2kReader",
   "type": "dynamicmodule",
   "ext" : nuke.PluginExt(),
   "srcs": glob.glob("src/j2kReader.cpp"),
   "custom": [freeimage.Require, nuke.Require]
  }
]

excons.DeclareTargets(env, prjs)
