from os.path import join, realpath
Import('env')

# private library flags
for item in env.get("CPPDEFINES", []):
    if isinstance(item, tuple) and item[0] == "LUOSHAL":
        print("Selected HAL for Luos and Robus is : %s" % item[1])
        env.Append(CPPPATH=[realpath("Network/Robus/HAL/" + item[1])])
        env.Replace(SRC_FILTER=["+<*.c>",
                                "+<../../../Network/Robus/src/*.c>",
                                "+<../../../Network/Robus/HAL/%s/*.c>" % item[1],
                                "+<../../Profiles/Core/*.c>",
                                "+<../../Profiles/State/*.c>",
                                "+<../../Profiles/Motor/*.c>",
                                "+<../../Profiles/Servo_motor/*.c>",
                                "+<../../Profiles/Voltage/*.c>",
                                "+<../../Bootloader/*.c>"])
        break
