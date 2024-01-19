import uuid
import sys
import subprocess
import platform as pf
from os import system, listdir, path, scandir, getcwd
import click
import json
import requests

from os.path import join, realpath
Import("env")

# Check if this script have been already executed during this compilation
visited_key = "__LUOS_CORE_SCRIPT_CALLED"
global_env = DefaultEnvironment()

if env.get("UPLOAD_PROTOCOL") == "custom" and not visited_key in global_env:
    click.secho("")
    click.secho("Luos engine build configuration:", underline=True)
    # install pyluos
    try:
        import pyluos
        # env.Execute(
        #     "$PYTHONEXE -m pip install pyluos --upgrade --quiet")
    except ImportError:  # module doesn't exist, deal with it.
        env.Execute("$PYTHONEXE -m pip install pyluos")
        pass
    try:
        from pyluos import version
        click.secho("\t* Pyluos revision " +
                    str(version.version) + " ready.", fg="green")
    except ImportError:  # module doesn't exist, deal with it.
        click.secho(
            "\t* Pyluos install failed. Platformio will be unable to use bootloader flash feature.", fg="red")

sources = ["+<*.c>",
           "+<../../IO/src/*.c>",
           "+<../../profiles/state/*.c>",
           "+<../../profiles/motor/*.c>",
           "+<../../profiles/servo_motor/*.c>",
           "+<../../profiles/voltage/*.c>",
           "+<../../bootloader/*.c>",]

# private library flags
find_HAL = False
env.Replace(SRC_FILTER=sources)
envdefs = env['CPPDEFINES'].copy()
for item in envdefs:
    if (isinstance(item, tuple) and item[0] == "LUOSHAL") and (find_HAL == False):
        find_HAL = True
        if (path.exists("engine/HAL/" + item[1])):
            if not visited_key in global_env:
                click.secho(
                    "\t* %s HAL selected for luos_engine." % item[1], fg="green")
                if (path.exists("engine/HAL/" + item[1] + "/hal_script.py")):
                    # This is an extra script dedicated to this HAL, run it
                    hal_script_path = realpath(
                        "engine/HAL/" + item[1] + "/hal_script.py")
                    env.SConscript(hal_script_path, exports="env")
        else:
            if not visited_key in global_env:
                click.secho("\t* %s HAL not found" % item[1], fg="red")
        env.Append(CPPPATH=[realpath("engine/HAL/" + item[1])])
        env.Append(SRC_FILTER=["+<../../HAL/%s/*.c>" % item[1]])

# Native only => we should put this on a specific script on engine/HAL/STUB
for item in env.ParseFlags(env['BUILD_FLAGS'])["CPPDEFINES"]:
    if (item == 'UNIT_TEST'):
        click.secho("")
        click.secho("Native unit testing:", underline=True)
        current_os = pf.system()
        click.secho("\t* Native Mock HAL for %s is selected for Luos." %
                    current_os, fg="green")
        env.Append(SRC_FILTER=["+<../../../test/_resources/*>"])
        for resources in scandir(getcwd() + "/test/_resources"):
            if resources.is_dir():
                env.Append(CPPPATH=[(resources.path)])

        if (current_os == 'Darwin'):
            env.Append(LINKFLAGS=["--coverage"])

        if (current_os == 'Linux') or (current_os == 'Windows'):
            env.Append(LINKFLAGS=["-lgcov", "--coverage", "-fprofile-arcs"])
        break

if not visited_key in global_env:
    if (find_HAL == False):
        click.echo(click.style("\t* No HAL selected. Please add a ", fg="red") + click.style(
            "-DLUOSHAL", fg="red", bold=True) + click.style(" compilation flag", fg="red"))

global_env[visited_key] = True
