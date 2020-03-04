Import('env')
from os.path import join, realpath

# private library flags
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "HAL":
        env.Append(CPPPATH=[realpath(join("hal", item[1]))])
        env.Replace(SRC_FILTER=["+<../src/*>", "-<../hal>", "+<%s>" % join("../hal", item[1])])
        break

# pass flags to a global build environment (for all libraries, etc)
global_env = DefaultEnvironment()
global_env.Append(
    CPPDEFINES=[
        ("MQTT_MAX_PACKET_SIZE", 512),
        "ARDUINOJSON_ENABLE_STD_STRING",
        ("BUFFER_LENGTH", 32)
    ]
)