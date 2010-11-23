import excons
from excons.tools import nuke, freeimage
import glob

env = excons.MakeBaseEnv()

prjs = [
  {"name": "jp2Reader",
   "type": "dynamicmodule",
   "ext" : nuke.PluginExt(),
   "srcs": ["src/jp2Reader.cpp"],
   "custom": [freeimage.Require, nuke.Require]
  },
  {"name": "j2kReader",
   "type": "dynamicmodule",
   "ext" : nuke.PluginExt(),
   "srcs": ["src/j2kReader.cpp"],
   "custom": [freeimage.Require, nuke.Require]
  },
  {"name": "jp2Writer",
   "type": "dynamicmodule",
   "ext" : nuke.PluginExt(),
   "srcs": ["src/jp2Writer.cpp"],
   "custom": [freeimage.Require, nuke.Require]
  },
  {"name": "j2kWriter",
   "type": "dynamicmodule",
   "ext" : nuke.PluginExt(),
   "srcs": ["src/j2kWriter.cpp"],
   "custom": [freeimage.Require, nuke.Require]
  }
]

excons.DeclareTargets(env, prjs)
