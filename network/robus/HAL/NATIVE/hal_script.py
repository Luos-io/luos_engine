#!/usr/bin/python
import os
from os import path
from os.path import realpath
import click
import time
Import("env")

global_env = DefaultEnvironment()

if not "MONGOOSE" in global_env:
    if (not path.exists("mongoose")):
        env.Execute(
            "git clone --depth 1 --branch 7.8 https://github.com/cesanta/mongoose.git")
    global_env["MONGOOSE"] = realpath("mongoose/")
env.Append(CPPPATH=[global_env["MONGOOSE"]])
env.Append(
    SRC_FILTER=["+<"+global_env["MONGOOSE"]+"/mongoose.c>"])
env.Append(CPPDEFINES=["MG_ENABLE_LINES=1"])
if os.name == 'nt':
    env.Append(CPPDEFINES=["_POSIX_C_SOURCE=200000L"])
    env.Append(LIBS=["ws2_32"])
click.secho("\t* mongoose 7.8 lib ready.", fg="green")
