#!/usr/bin/python
# This sript is used to modify the platformio build environment to use cheerp instead

import os
import platform
# Local env
Import("env")
 # Global env, use it if you call this script from a library.json file
genv = DefaultEnvironment()

 # Get the cheerp bin folder
cheerp_bin_path = None
if  platform.system() == 'Windows':
    cheerp_bin_path = 'c:/cheerp/bin'
if platform.system() == 'Linux':
    cheerp_bin_path = '/opt/cheerp/bin'
if platform.system() == 'Darwin':
    cheerp_bin_path = '/Applications/cheerp/bin'

if platform.system() == 'Darwin':
    # Create a symlink to Cheerp clang++ in gcc
    try:
        os.symlink(cheerp_bin_path + '/clang++', cheerp_bin_path + '/gcc')
    except FileExistsError:
        pass

    # Create a symlink to Cheerp clang++ in g++
    try:
        os.symlink(cheerp_bin_path + '/clang++', cheerp_bin_path + '/g++')
    except FileExistsError:
        pass

# Add the cheerp fake gcc path at the beginning of the PATH environment variable than platformio will use it by default
current_path = os.environ.get('PATH', '')
os.environ['PATH'] = f"{cheerp_bin_path}{os.pathsep}{current_path}"

# Print the path used when calling gcc
print("GCC used by platformio is :" + os.popen("which gcc").read())

for e in [env, genv]:
    # Add the cheerp-wasm target to the compiler flags
    e.Append(CCFLAGS=["--target=cheerp-wasm"])

    # Add the cheerp-wasm target to the linker flags
    e.Append(LINKFLAGS=["--target=cheerp-wasm"])

    # Replace the ar and ranlib commands with the appropriate llvm-ar command
    e.Replace(AR=cheerp_bin_path + "/llvm-ar",
            RANLIB=cheerp_bin_path + "/llvm-ar s")

    # Replace the output filename with the appropriate extension
    e.Replace(PROGNAME="program.bc")

