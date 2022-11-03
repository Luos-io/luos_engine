#!/usr/bin/python
import os
from os import path
from os.path import realpath
import click
Import("env")


if (not path.exists("mongoose")):
    env.Execute(
        "git clone --depth 1 --branch 7.8 https://github.com/cesanta/mongoose.git")

env.Append(CPPPATH=[realpath("mongoose/")])
env.Append(SRC_FILTER=["+<WS/native/mongoose/mongoose.c>"])
env.Append(CPPDEFINES=["MG_ENABLE_LINES=1"])
if os.name == 'nt':
    env.Append(CPPDEFINES=["_POSIX_C_SOURCE=200000L"])
    env.Append(LIBS=["ws2_32"])
click.secho("\t* mongoose 7.8 lib ready.", fg="green")
