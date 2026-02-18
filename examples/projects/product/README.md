<a href="https://luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/603e0cc45afbb50963aa85f2_Gif%20noir%20rect.gif" alt="Luos logo" title="Luos" align="right" height="100" /></a>

![](https://github.com/Luos-io/luos_engine/actions/workflows/build.yml/badge.svg)
[![](https://img.shields.io/github/license/Luos-io/luos_engine)](https://github.com/Luos-io/luos_engine/blob/master/LICENSE)

[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://www.luos.io)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/luos/library/luos_engine.svg)](https://registry.platformio.org/libraries/luos_engine/luos_engine)

[![](https://img.shields.io/discord/902486791658041364?label=Discord&logo=discord&style=social)](https://discord.gg/luos)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)

# The complete product example :bulb:

This example demonstrates how to deal with a real life project using Luos_engine. This code folder deals with custom types, custom messages commands, and demonstrate how to adapt your gate and pyluos to properly handle it.

This project is a laser Galvo controller example that can be used in an engraving machine, a small surface laser cutter or a laser show device.
This has been tested with the great [interface board made by the opengalvo OPAL project](https://github.com/leswright1977/OPAL_PCB) on STM32-L476RG and a simple cat laser pointer toy.

![The demonstration of a running galvo with a nucleo-L476RG.](galvo_demo.gif)

This product is composed of multiple nodes:

- A gate node called `custom_gate` running on your computer
- A laser Galvo node called `laser_galvo` running on a microcontroller (tested on STM32-L476RG, but it should work on most microcontrollers within the STM32 family).

In this folder, you can also find a Python notebook `laser_galvo_control.ipynb` to control the laser Galvo.

To learn more about how to deal with complete product project with Luos please read our [code organization documentation](https://www.luos.io/docs/luos-technology/basics/organization).

## How to use :computer:

1.  Download and install [Platformio](https://platformio.org/platformio-ide)
2.  Open the `custom_gate` folder into Platformio
3.  Build (Platformio will do the rest)
4.  Open the `laser_galvo` folder into Platformio
5.  Build and flash the board (Platformio will do the rest) (of course your board with the galvo should be connected to your computer)
6.  Run the `custom_gate` program in `custom_gate/.pio/build/native_serial/program` (or `custom_gate/.pio/build/native_serial/program.exe` if you use windows)
7.  Install python requirements with `pip install -r requirements.txt`
8.  Then you can play with the Ipython notebook `laser_galvo_control.ipynb` to control the laser Galvo

## Don't hesitate to read [our documentation](https://www.luos.io/docs/luos-technology), or to post your questions/issues on the [Luos' community](https://discord.gg/luos). :books:

[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://www.luos.io)
[![](https://img.shields.io/badge/LinkedIn-Follow%20us-0077B5?style=flat&logo=linkedin)](https://www.linkedin.com/company/luos)
