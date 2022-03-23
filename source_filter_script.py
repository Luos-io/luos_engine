import sys
import subprocess
from os import system, listdir, path
from os.path import join, realpath
Import('env')

# install pyluos
try:
    import pyluos
    subprocess.check_call([sys.executable, "-m", "pip",
                          "install", "pyluos", "--upgrade", "--quiet"])
except ImportError:  # module doesn't exist, deal with it.
    subprocess.check_call([sys.executable, "-m", "pip", "install", "pyluos"])
    pass
print('\n\033[4mLuos engine build configuration:\033[0m\n')
try:
    from pyluos import version
    print("\t*\033[0;32m Pyluos revision " +
          str(version.version) + " ready.\033[0m")
except ImportError:  # module doesn't exist, deal with it.
    print("\t*\033[0;32m Pyluos install failed. Platformio will be unable to use bootloader flash feature.\033[0m")

sources = ["+<*.c>",
           "+<../../../Network/Robus/src/*.c>",
           "+<../../Profiles/Core/*.c>",
           "+<../../Profiles/State/*.c>",
           "+<../../Profiles/Motor/*.c>",
           "+<../../Profiles/Servo_motor/*.c>",
           "+<../../Profiles/Voltage/*.c>",
           "+<../../Bootloader/*.c>"]

# private library flags
find_HAL = False
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "LUOSHAL":
        find_HAL = True
        if (path.exists("Network/Robus/HAL/" + item[1]) and path.exists("Engine/HAL/" + item[1])):
            print(
                "\t*\033[0;32m %s HAL selected for Luos and Robus.\033[0m\n" % item[1])
        else:
            print("\t*\033[1;31m %s HAL not found\033[0m\n" % item[1])
        env.Append(CPPPATH=[realpath("Network/Robus/HAL/" + item[1])])
        env.Append(CPPPATH=[realpath("Engine/HAL/" + item[1])])
        env.Replace(SRC_FILTER=sources)
        env.Append(
            SRC_FILTER=["+<../../../Network/Robus/HAL/%s/*.c>" % item[1]])
        env.Append(SRC_FILTER=["+<../../HAL/%s/*.c>" % item[1]])
        break

if (find_HAL == False):
    print("\033[1;31mNo HAL selected. Please add a \033[0;30;47m-DLUOSHAL\033[0m\033[1;31m compilation flag\033[0m\n")

# native unit testing
if (env.get("PIOPLATFORM") == "native"):
    for item in env.get("CPPDEFINES", []):
        if (item == 'UNIT_TEST'):
            # Flags for unit testing
            env.Append(LINKFLAGS=["--coverage"])
            env.Replace(SRC_FILTER=sources)
            env.Append(SRC_FILTER=["+<../../../test/_resources/*>"])

            def generateCoverageInfo(source, target, env):
                for file in os.listdir("test"):
                    os.system(".pio/build/native/program test/"+file)
                os.system("lcov -d .pio/build/native/ -c -o lcov.info")
                os.system(
                    "lcov --remove lcov.info '*/tool-unity/*' '*/test/*' -o filtered_lcov.info")
                os.system("genhtml -o cov/ --demangle-cpp filtered_lcov.info")

            # Generate code coverage when testing workflow is ended
            env.AddPostAction(".pio/build/native/program",
                              generateCoverageInfo)
            break
