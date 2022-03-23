from os import system, listdir
from os.path import join, realpath
Import('env')

sources = ["+<*.c>",
           "+<../../../Network/Robus/src/*.c>",
           "+<../../Profiles/Core/*.c>",
           "+<../../Profiles/State/*.c>",
           "+<../../Profiles/Motor/*.c>",
           "+<../../Profiles/Servo_motor/*.c>",
           "+<../../Profiles/Voltage/*.c>",
           "+<../../Bootloader/*.c>"]

# private library flags
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "LUOSHAL":
        print("Selected HAL for Luos and Robus is : %s" % item[1])
        env.Append(CPPPATH=[realpath("Network/Robus/HAL/" + item[1])])
        env.Append(CPPPATH=[realpath("Engine/HAL/" + item[1])])
        env.Replace(SRC_FILTER=sources)
        env.Append(
            SRC_FILTER=["+<../../../Network/Robus/HAL/%s/*.c>" % item[1]])
        env.Append(SRC_FILTER=["+<../../HAL/%s/*.c>" % item[1]])
        break

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
