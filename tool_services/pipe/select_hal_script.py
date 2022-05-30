from os import path
import click
from os.path import join, realpath
Import('env')

click.secho("")
click.secho("Pipe build configuration:", underline=True)
# private library flags
find_mode = False
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "PIPEMODE":
        find_mode = True
        if (path.exists(item[1])):
            click.secho("\t* %s mode selected." %
                        item[1], fg="green")
            # now find the hal
            find_HAL = False
            for itemhal in env.get("CPPDEFINES", []):
                if isinstance(itemhal, tuple) and itemhal[0] == "PIPEHAL":
                    find_HAL = True
                    if (path.exists(item[1] + "/" + itemhal[1])):
                        click.secho(
                            "\t* %s HAL selected." % itemhal[1], fg="green")
                    else:
                        click.secho("\t* %s HAL not found." %
                                    item[1], fg="red")
                    env.Append(CPPPATH=[realpath(item[1] + "/" + itemhal[1])])
                    env.Replace(
                        SRC_FILTER=["+<*.c>, +<%s/%s>" % (item[1], itemhal[1])])
                    break
            if find_HAL == False:
                click.secho(
                    "\t* No PIPEHAL selected, please add a -D PIPEHAL definition.", fg="red")
        else:
            click.secho("\t* %s mode not found." %
                        item[1], fg="red")
            exit()
        break

if find_mode == False:
    click.secho(
        "\t* No PIPEMODE selected, please add a -D PIPEMODE definition.", fg="red")
