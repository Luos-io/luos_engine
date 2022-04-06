import sys
import subprocess
import platform as pf
from os import system, listdir, path, scandir, getcwd

from os.path import join, realpath
Import('env')

# Check if this script have been already executed during this compilation
visited_key = "__LUOS_CORE_SCRIPT_CALLED"
global_env = DefaultEnvironment()

if not visited_key in global_env:
    # install pyluos
    try:
        import pyluos
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", "pyluos", "--upgrade", "--quiet"])
    except ImportError:  # module doesn't exist, deal with it.
        subprocess.check_call(
            [sys.executable, "-m", "pip", "install", "pyluos"])
        pass
    print('\n\033[4mLuos engine build configuration:\033[0m\n')
    try:
        from pyluos import version
        print("\t*\033[0;32m Pyluos revision " +
              str(version.version) + " ready.\033[0m")
    except ImportError:  # module doesn't exist, deal with it.
        print(
            "\t*\033[0;32m Pyluos install failed. Platformio will be unable to use bootloader flash feature.\033[0m")

sources = ["+<*.c>",
           "+<../../../network/robus/src/*.c>",
           "+<../../profiles/core/*.c>",
           "+<../../profiles/state/*.c>",
           "+<../../profiles/motor/*.c>",
           "+<../../profiles/servo_motor/*.c>",
           "+<../../profiles/voltage/*.c>",
           "+<../../bootloader/*.c>"]

# private library flags
find_HAL = False
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "LUOSHAL":
        find_HAL = True
        if (path.exists("network/robus/HAL/" + item[1]) and path.exists("engine/HAL/" + item[1])):
            if not visited_key in global_env:
                print(
                    "\t*\033[0;32m %s HAL selected for Luos and Robus.\033[0m\n" % item[1])
        else:
            if not visited_key in global_env:
                print("\t*\033[1;31m %s HAL not found\033[0m\n" % item[1])
        env.Append(CPPPATH=[realpath("network/robus/HAL/" + item[1])])
        env.Append(CPPPATH=[realpath("engine/HAL/" + item[1])])
        env.Replace(SRC_FILTER=sources)
        env.Append(
            SRC_FILTER=["+<../../../network/robus/HAL/%s/*.c>" % item[1]])
        env.Append(SRC_FILTER=["+<../../HAL/%s/*.c>" % item[1]])
        break


# native unit testing
find_MOCK_HAL = False
for item in env.get("CPPDEFINES", []):
    if not "native" in env.get("PIOPLATFORM"):
        break
    if (item == 'UNIT_TEST'):
        current_os = pf.system()
        if find_MOCK_HAL == False:
            print(
                "\t*\033[0;32m Mock HAL for %s is selected for Luos and Robus.\033[0m\n" % current_os)
        find_MOCK_HAL = True
        find_HAL = True
        env.Replace(SRC_FILTER=sources)
        env.Append(SRC_FILTER=["-<test/>"])
        env.Append(SRC_FILTER=["+<../../../test/_resources/*>"])

        for resources in scandir(getcwd() + "/test/_resources"):
            if resources.is_dir():
                env.Append(CPPPATH=[(resources.path)])

        if (current_os == 'Linux') or (current_os == 'Darwin'):
            env.Append(LINKFLAGS=["-m32"])
        elif current_os == 'Windows':
            env.Append(LINKFLAGS=["-lgcov"])
            env.Append(LINKFLAGS=["--coverage"])
            env.Append(LINKFLAGS=["-fprofile-arcs"])

            def generateCoverageInfo(source, target, env):
                for file in os.listdir("test"):
                    os.system(".pio/build/native/program test/"+file)
                os.system("lcov -d .pio/build/native/ -c -o lcov.info")
                os.system(
                    "lcov --remove lcov.info '*/tool-unity/*' '*/test/*' -o filtered_lcov.info")
                os.system("genhtml -o cov/ --demangle-cpp filtered_lcov.info")

            # Generate code coverage when testing workflow is ended
            # CODE COVERAGE WILL BE ADDED SOON
            #env.AddPostAction(".pio/build/native/program", generateCoverageInfo)
        else:
            print("Unit tests are not supported on your os ", current_os)
        break

if not visited_key in global_env:
    if (find_HAL == False):
        print("\033[1;31mNo HAL selected. Please add a \033[0;30;47m-DLUOSHAL\033[0m\033[1;31m compilation flag\033[0m\n")

global_env[visited_key] = True
