<a href="https://luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/603e0cc45afbb50963aa85f2_Gif%20noir%20rect.gif" alt="Luos logo" title="Luos" align="right" height="100" /></a>

[![](https://img.shields.io/github/license/Luos-io/luos_engine)](https://github.com/Luos-io/luos_engine/blob/master/LICENSE)

[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://www.luos.io)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/luos/library/Pipe.svg)](https://registry.platformio.org/libraries/luos_engine/Pipe)

[![](https://img.shields.io/discord/902486791658041364?label=Discord&logo=discord&style=social)](http://bit.ly/JoinLuosDiscord)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)

# Pipe Driver

The pipe driver allow you to share a communication way to the outside world with the other services of your device.

# Pipe configuration

To make this driver work you need to select the HAL corresponding to your needs. To select it you just have to compile the good file by selecting the good folder

## Platformio configuration

If you are using Platformio we already made a script selecting and compiling the files for you depending on some build flags you give him.
To configure it you have to setup `PIPEMODE`and `PIPEHAL` flags.
For example in your platformio.ini

```JSON
...
build_flags =
    -include node_config.h
    -D LUOSHAL=ATSAMD21_ARDUINO
    -D PIPEMODE=SERIAL
    -D PIPEHAL=ARDUINO
...
```

Here is the configuration choices you have :

| PIPEHAL     | PIPEMODE=SERIAL | PIPEMODE=BLE | PIPEMODE=WIFI |
| ----------- | --------------- | ------------ | ------------- |
| ARDUINO     | ✅              |              |               |
| NUCLEO-F0   | ✅              |              |               |
| NUCLEO-F4   | ✅              |              |               |
| NUCLEO-G431 | ✅              |              |               |
| NUCLEO-G474 | ✅              |              |               |
| NUCLEO-L4   | ✅              |              |               |

## To learn more

This section details the features of Luos technology as an embedded development platform, following these subjects:

- Let's test through the [Luos get started](https://www.luos.io/tutorials/get-started), to build, flash, run, and control your very first Luos code.
- The [Basics of Luos](https://www.luos.io/docs/luos-technology/basics), explaining the general concepts and the project organization.
- Definition of [Nodes](https://www.luos.io/docs/luos-technology/node), and the relation between Luos and the physical world.
- Definition of [Packages](https://www.luos.io/docs/luos-technology/package), and how to make a portable and reusable development.
- Definition of [Services](https://www.luos.io/docs/luos-technology/services), how to create and declare features in your product.
- Definition of [Messages](https://www.luos.io/docs/luos-technology/message), when, why, and how to handle them, explaining the more advanced features of Luos.
