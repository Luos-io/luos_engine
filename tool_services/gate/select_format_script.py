
from os import path
import click
from os.path import join, realpath
Import('env')

click.secho("")
click.secho("Gate build configuration:", underline=True)
# private library flags
find_format = False
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "GATEFORMAT":
        find_format = True
        if (path.exists(item[1])):
            click.secho("\t* %s translation format selected." %
                        item[1], fg="green")
            env.Append(CPPPATH=[realpath(item[1])])
            env.Replace(SRC_FILTER=["+<*.c>, +<%s>" % item[1]])
        elif (path.exists(env.get("PROJECT_DIR")+"/lib/"+item[1])):
            click.secho("\t* %s translation format selected." %
                        item[1], fg="green")
            env.Append(
                CPPPATH=[realpath(env.get("PROJECT_DIR")+"/lib/"+item[1])])
            env.Replace(SRC_FILTER=["+<*.c>, +<%s>" %
                        env.get("PROJECT_DIR")+"/lib/"+item[1]])
        else:
            click.secho("\t* %s translation format not found." %
                        item[1], fg="red")
        break

if find_format == False:
    click.secho(
        "\t* No GATEFORMAT selected, please add a -D GATEFORMAT definition.", fg="red")
