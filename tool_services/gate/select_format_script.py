
from os import path
import click
from os.path import join, realpath
Import('env')

# private library flags
find_format = False
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "GATEFORMAT":
        find_format = True
        if (path.exists(item[1])):
            click.secho("\t* %s translation format selected for Gate.\n" %
                        item[1], fg="green")
        else:
            click.secho("\t* %s translation format for Gate not found\n" %
                        item[1], fg="red")
        env.Append(CPPPATH=[realpath(item[1])])
        env.Replace(SRC_FILTER=["+<*.c>, +<%s>" % item[1]])
        break
