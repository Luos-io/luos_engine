<a href="https://www.luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/603e0cc45afbb50963aa85f2_Gif%20noir%20rect.gif" alt="Luos logo" title="Luos" align="right" height="100" /></a>

![](https://github.com/Luos-io/luos_engine/actions/workflows/build.yml/badge.svg)
[![](https://img.shields.io/github/license/Luos-io/luos_engine)](https://github.com/Luos-io/luos_engine/blob/master/LICENSE)

[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://www.luos.io)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/luos/library/luos_engine.svg)](https://registry.platformio.org/libraries/luos_engine/luos_engine)

[![](https://img.shields.io/discord/902486791658041364?label=Discord&logo=discord&style=social)](http://bit.ly/JoinLuosDiscord)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)

# Luos engine HAL

This folder regroups all of the HAL (Hardware Abstraction Layers) routines usable with Luos engine.
There is also HAL depending on your network on the `Network` folder
You can choose the one you need for your microcontroller or create your own following our templates and guidelines.
Do not hesitate to share your project and ask us about features on [our Discord](http://bit.ly/JoinLuosDiscord).

## Compatible MCU

### STM32F0 family:

- STM32F0x0
- STM32F0x1
- STM32F0x2 -> LuosHAL default configuration on NUCLEO-F072RB
- STM32F0x8

### STM32F4 family:

- STM32F401
- STM32F405/415
- STM32F407/417
- STM32F410 -> LuosHAL default configuration on NUCLEO-F410RB
- STM32F411
- STM32F412
- STM32F413/423
- STM32F427/437
- STM32F429/439
- STM32F446
- STM32F469/479

### STM32G4 family:

- STM32G4x1-> LuosHAL default configuration on NUCLEO-G431KB
- STM32G4x3
- STM32G4x4

### STM32L4 family:

- STM32L4x1
- STM32L4x2-> LuosHAL default configuration on NUCLEO-L432KC
- STM32L4x3
- STM32L4x5
- STM32L4x6

### ATSAMD family:

- ATSAMD21Exx
- ATSAMD21Gxx
- ATSAMD21Jxx -> LuosHAL default configuration on SAMDJ18A

### ARDUINO family:

- ARDUINO_SAMD_ZERO -> LuosHAL default configuration on ARDUINO ZERO
- ARDUINO_SAMD_MKRZERO
- ARDUINO_SAMD_MKR1000
- ARDUINO_SAMD_MKRWIFI1010
- ARDUINO_SAMD_MKRFox1200
- ARDUINO_SAMD_MKRWAN1300
- ARDUINO_SAMD_MKRWAN1310
- ARDUINO_SAMD_MKRGSM1400
- ARDUINO_SAMD_MKRNB1500
- ARDUINO_SAMD_NANO_33_IOT
- ARDUINO_SAMD_MKRVIDOR4000

## Don't hesitate to read [our documentation](https://www.luos.io/docs/luos-technology), or to post your questions/issues on the [Luos' community](https://www.luos.io/community). :books:
