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

    if libPath is None:
        return

    # Convert the luos_engine.a archive to a shared library.
    # A static archive is only searched for already-referenced symbols, so
    # linking it into an otherwise empty shared lib yields zero symbols.
    # Force the linker to include every object from the archive.
    if (pf.system() == 'Windows'):
        env.Execute("gcc -shared -fPIC -o $BUILD_DIR/libluos_engine.dll -Wl,--whole-archive " + libPath + " -Wl,--no-whole-archive")
        click.secho("* Luos engine shared library available in " + str(env.subst("$BUILD_DIR")) + "/libluos_engine.dll .", fg="green")
        return

    if (pf.system() == 'Linux'):
        env.Execute("gcc -shared -fPIC -o $BUILD_DIR/libluos_engine.so -Wl,--whole-archive " + libPath + " -Wl,--no-whole-archive")
        suffix = ".so"
        force_load = lambda a: "-Wl,--whole-archive " + a + " -Wl,--no-whole-archive"
        unresolved_ok = "-Wl,--unresolved-symbols=ignore-in-object-files"
        dyn_flag = "-shared"
    elif (pf.system() == 'Darwin'):
        env.Execute("gcc -dynamiclib -fPIC -fvisibility=default -install_name $BUILD_DIR -o $BUILD_DIR/libluos_engine.dylib -Wl,-force_load," + libPath)
        suffix = ".dylib"
        force_load = lambda a: "-Wl,-force_load," + a
        unresolved_ok = "-Wl,-undefined,dynamic_lookup"
        dyn_flag = "-dynamiclib"
    else:
        return

    click.secho("\n")
    click.secho("Luos engine shared libraries available in " + str(env.subst("$BUILD_DIR")) + "/ :", underline=True)
    click.secho("\t* libluos_engine" + suffix, fg="green")
    if not networklibs:
        click.secho("\t(no network archives found — per-phy dylibs skipped)", fg="yellow")
    for networklib in networklibs:
        base = os.path.basename(networklib)[3:-2]  # strip "lib" prefix and ".a"
        out = "$BUILD_DIR/lib" + base + suffix
        name_arg = (" -install_name @rpath/lib" + base + suffix + " ") if pf.system() == 'Darwin' else " "
        env.Execute(
            "gcc " + dyn_flag + " -fPIC -fvisibility=default"
            + name_arg
            + "-o " + out + " "
            + force_load(networklib) + " "
            + unresolved_ok
        )
        click.secho("\t* lib" + base + suffix, fg="green")

env.AddPostAction("$PROGPATH", shared_lib)
env.Append(LINKFLAGS=["-fPIC", "-fvisibility=default"])
env.Append(BUILD_FLAGS=["-fPIC", "-fvisibility=default"])
