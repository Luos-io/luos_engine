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

telemetry = True
luos_telemetry = {"telemetry_type": "luos_engine_build",
                  "mac": hex(uuid.getnode()),
                  "system": sys.platform,
                  "unix_time": env.get("UNIX_TIME"),
                  "platform": env.get("PIOPLATFORM"),
                  "project_path": env.get("PROJECT_DIR")}

if (env.get("BOARD_MCU") != None):
    luos_telemetry["mcu"] = env.get("BOARD_MCU")
if (env.get("BOARD_F_CPU") != None):
    luos_telemetry["f_cpu"] = env.get("BOARD_F_CPU")

try:
    luos_telemetry["framework"] = env.get("PIOFRAMEWORK")[0]
except:
    pass

# Check if this script have been already executed during this compilation
visited_key = "__LUOS_CORE_SCRIPT_CALLED"
global_env = DefaultEnvironment()

if not visited_key in global_env:
    click.secho("")
    click.secho("Luos engine build configuration:", underline=True)
    # install pyluos
    try:
        import pyluos
        env.Execute("$PYTHONEXE -m pip install pyluos --upgrade --quiet")
    except ImportError:  # module doesn't exist, deal with it.
        env.Execute("$PYTHONEXE -m pip install pyluos")
        pass
    try:
        from pyluos import version
        click.secho("\t* Pyluos revision " +
                    str(version.version) + " ready.", fg="green")
        luos_telemetry["pyluos_rev"] = str(version.version)
    except ImportError:  # module doesn't exist, deal with it.
        click.secho(
            "\t* Pyluos install failed. Platformio will be unable to use bootloader flash feature.", fg="red")
        luos_telemetry["pyluos_rev"] = "none"

sources = ["+<*.c>",
           "+<../../../network/robus/src/*.c>",
           "+<../../IO/src/*.c>",
           "+<../../../network/robus/selftest/*.c>",
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
        if (path.exists("network/robus/HAL/" + item[1]) and path.exists("engine/HAL/" + item[1])):
            if not visited_key in global_env:
                click.secho(
                    "\t* %s HAL selected for Luos and Robus." % item[1], fg="green")
                luos_telemetry["luos_hal"] = item[1]
                if (path.exists("network/robus/HAL/" + item[1] + "/hal_script.py")):
                    # This is an extra script dedicated to this HAL, run it
                    hal_script_path = realpath(
                        "network/robus/HAL/" + item[1] + "/hal_script.py")
                    env.SConscript(hal_script_path, exports="env")
                if (path.exists("engine/HAL/" + item[1] + "/hal_script.py")):
                    # This is an extra script dedicated to this HAL, run it
                    hal_script_path = realpath(
                        "engine/HAL/" + item[1] + "/hal_script.py")
                    env.SConscript(hal_script_path, exports="env")
        else:
            if not visited_key in global_env:
                click.secho("\t* %s HAL not found" % item[1], fg="red")
                luos_telemetry["luos_hal"] = "invalid" + str(item[1])
        env.Append(CPPPATH=[realpath("network/robus/HAL/" + item[1])])
        env.Append(CPPPATH=[realpath("engine/HAL/" + item[1])])
        env.Append(
            SRC_FILTER=["+<../../../network/robus/HAL/%s/*.c>" % item[1]])
        env.Append(SRC_FILTER=["+<../../HAL/%s/*.c>" % item[1]])
    if (item == 'NOTELEMETRY'):
        telemetry = False


if not visited_key in global_env:
    if (telemetry == True):
        click.secho("\t* Telemetry enabled.", fg="green")
        try:
            r = requests.post("https://monorepo-services.vercel.app/api/telemetry",
                              data=luos_telemetry)
            if not r:
                click.secho("\tX Telemetry request failed : error " +
                            str(r.status_code), fg="red")
        except:
            click.secho("\tX Telemetry request failed.", fg="red")
    else:
        click.secho(
            "\t* Telemetry disabled, please consider enabling it by removing the 'NOTELEMETRY' flag to help Luos_engine improve.", fg="red")
    click.secho("")

# Native only => we should put this on a specific script on engine/HAL/STUB
for item in env.ParseFlags(env['BUILD_FLAGS'])["CPPDEFINES"]:
    if (item == 'UNIT_TEST'):
        click.secho("Native unit testing:", underline=True)
        current_os = pf.system()
        click.secho("\t* Native Mock HAL for %s is selected for Luos and Robus." %
                    current_os, fg="green")
        env.Append(SRC_FILTER=["+<../../../test/_resources/*>"])
        for resources in scandir(getcwd() + "/test/_resources"):
            if resources.is_dir():
                env.Append(CPPPATH=[(resources.path)])

        # CODE COVERAGE WILL BE ADDED SOON
        # if (current_os == 'Linux') or (current_os == 'Darwin') or (current_os == 'Windows'):
        #     env.Append(LINKFLAGS=["-lgcov"])
        #     env.Append(LINKFLAGS=["--coverage"])
        #     env.Append(LINKFLAGS=["-fprofile-arcs"])

        #     def generateCoverageInfo(source, target, env):
        #         for file in os.listdir("test"):
        #             env.Execute(".pio/build/native/program test/"+file)
        #         env.Execute("lcov -d .pio/build/native/ -c -o lcov.info")
        #         env.Execute(
        #             "lcov --remove lcov.info '*/tool-unity/*' '*/test/*' -o filtered_lcov.info")
        #         env.Execute(
        #             "genhtml -o cov/ --demangle-cpp filtered_lcov.info")

        # Generate code coverage when testing workflow is ended
        # env.AddPostAction(".pio/build/native/program", generateCoverageInfo)
        # else:
        #     click.echo("Unit tests are not supported on your os ", current_os)
        break

if not visited_key in global_env:
    if (find_HAL == False):
        click.echo(click.style("\t* No HAL selected. Please add a ", fg="red") + click.style(
            "-DLUOSHAL", fg="red", bold=True) + click.style(" compilation flag", fg="red"))

global_env[visited_key] = True
