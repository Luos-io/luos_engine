from os.path import join, realpath
Import('env')

# private library flags
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "GATEFORMAT":
        env.Append(CPPPATH=[realpath(item[1])])
        env.Replace(SRC_FILTER=["+<*.c>, +<%s>" % item[1]])
        break