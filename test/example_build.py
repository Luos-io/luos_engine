#!/usr/bin/env python
from os.path import join, realpath, exists
import os
import subprocess
import click
import multiprocessing as mp
import sys
import time
import argparse


exampledir = 'examples/projects'

# Compute the number of examples
nb_example = 0
compiled_example = None
examples_success = None
examples_failed = None

for target in os.listdir(exampledir):
    examples_path = join(realpath(exampledir), target)
    if os.path.isdir(examples_path):
        for example in os.listdir(os.path.join(exampledir, target)):
            example_path = join(realpath(exampledir), target, example)
            if (exists(join(example_path, "platformio.ini"))):
                # This is a valid example, count it
                nb_example += 1


def init(compiled, success, failed):
    ''' store the counter for later use '''
    global compiled_example
    global examples_success
    global examples_failed
    compiled_example = compiled
    examples_success = success
    examples_failed = failed


# Example compilation task
OKGREEN = '\r\033[92m'
FAIL = '\r\033[91m'
ENDC = '\033[0m'


def compile_example(cmd, target, example, clean):
    global compiled_example
    if clean:
        subprocess.call(cmd + " --target clean", shell=True,
                        stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb'))
    if subprocess.call(cmd, shell=True, stdout=open(os.devnull, 'wb'), stderr=open(os.devnull, 'wb')):
        with compiled_example.get_lock():
            compiled_example.value += 1
        value = FAIL+"FAILED          " + str(join(target, example) + ENDC)
        print(value, flush=True)
        with examples_failed.get_lock():
            examples_failed.value += 1
    else:
        with compiled_example.get_lock():
            compiled_example.value += 1
        value = OKGREEN + "SUCCESS         " + \
            str(join(target, example)) + ENDC
        print(value, flush=True)
        with examples_success.get_lock():
            examples_success.value += 1
    return True


if __name__ == '__main__':

    ## Parse arguments ##
    parser = argparse.ArgumentParser(description='A command to build them all as fast as possible!\n',
                                     formatter_class=argparse.RawTextHelpFormatter)

    # General arguments
    parser.add_argument("--clean", action="store_true",
                        help="Clean all examples before building them")
    args = parser.parse_args()

    start = time.time()
    # Create all example compilation tasks
    compiled_example = mp.Value('i', 0)
    examples_success = mp.Value('i', 0)
    examples_failed = mp.Value('i', 0)
    pool = mp.Pool(nb_example, initializer=init, initargs=(compiled_example,
                                                           examples_success,
                                                           examples_failed,))
    click.secho(
        "\nbuild result    Project name\n------------    ------------")
    for target in os.listdir(exampledir):
        examples_path = join(realpath(exampledir), target)
        if os.path.isdir(examples_path):
            for example in os.listdir(os.path.join(exampledir, target)):
                example_path = join(exampledir, target, example)
                cmd = "platformio run -d " + example_path
                if (exists(join(example_path, "platformio.ini"))):
                    pool.apply_async(
                        compile_example, args=(cmd, target, example, args.clean))

    pool.close()
    while compiled_example.value < nb_example:
        # Print a nice loading bar
        chars = "/—\|"
        for char in chars:
            sys.stdout.write(
                '\r'+'Building ' + char + ' (' + str(compiled_example.value) + "/" + str(nb_example) + ")")
            time.sleep(.1)
            sys.stdout.flush()

    pool.join()
    print("\r--------------------------------------------\nBuild summary\n--------------------------------------------")
    print("\t- Success\t\t\t" + str(examples_success.value) + "/" + str(nb_example))
    print("\t- Failed\t\t\t" + str(examples_failed.value) + "/" + str(nb_example))
    print("\t- Total compilation time\t" + str(time.time() - start) + "s")
