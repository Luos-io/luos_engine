Import("env")
import os
import platform as pf
import click

def shared_lib(source, target, env):
    # Try to find the luos_engine.a somwhere on $BUILD_DIR/*/ archive and save it to a libPath variable
    libPath = None
    for root, dirs, files in os.walk(env.subst("$BUILD_DIR")):
        for file in files:
            if file.endswith("luos_engine.a"):
                libPath = os.path.join(root, file)
                break
        if libPath is not None:
            break
    # Try to find all the network libs
    networklibs = []
    for root, dirs, files in os.walk(env.subst("$BUILD_DIR")):
        for file in files:
            if file.endswith("network.a"):
                networklibs.append(os.path.join(root, file))
                break

    if libPath is not None:
        # Convert the luos_engine.a archive to a shared library.
        # A static archive is only searched for already-referenced symbols, so
        # linking it into an otherwise empty shared lib yields zero symbols.
        # Force the linker to include every object from the archive.
        if (pf.system() == 'Windows'):
            env.Execute("gcc -shared -fPIC -o $BUILD_DIR/libluos_engine.dll -Wl,--whole-archive " + libPath + " -Wl,--no-whole-archive")
            click.secho("* Luos engine shared library available in " + str(env.subst("$BUILD_DIR")) + "/libluos_engine.dll .", fg="green")
        elif (pf.system() == 'Linux'):
            env.Execute("gcc -shared -fPIC -o $BUILD_DIR/libluos_engine.so -Wl,--whole-archive " + libPath + " -Wl,--no-whole-archive")
            click.secho("* Luos engine shared library available in " + str(env.subst("$BUILD_DIR")) + "/libluos_engine.so .", fg="green")
        elif (pf.system() == 'Darwin'):
            env.Execute("gcc -dynamiclib -fPIC -fvisibility=default -install_name $BUILD_DIR -o $BUILD_DIR/libluos_engine.dylib -Wl,-force_load," + libPath)
            click.secho("\n")
            click.secho("Luos engine shared libraries available in " + str(env.subst("$BUILD_DIR")) + "/ :", underline=True)
            for networklib in networklibs:
                click.secho("\t* " + os.path.basename(networklib)[0:-10] + "_luos_engine.dylib ", fg="green")

env.AddPostAction("$PROGPATH", shared_lib)
env.Append(LINKFLAGS=["-fPIC", "-fvisibility=default"])
env.Append(BUILD_FLAGS=["-fPIC", "-fvisibility=default"])
