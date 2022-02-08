Import("env")
import os

env.Append(LINKFLAGS=["--coverage"])

def generateCoverageInfo(source, target, env):
    for file in os.listdir("test"):
        os.system(".pio/build/native/program test/"+file)
    os.system("lcov -d .pio/build/native/ -c -o lcov.info")
    os.system("lcov --remove lcov.info '*/tool-unity/*' '*/test/*' -o filtered_lcov.info")
    os.system("genhtml -o cov/ --demangle-cpp filtered_lcov.info")

env.AddPostAction(".pio/build/native/program", generateCoverageInfo)
