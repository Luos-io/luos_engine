<a href="https://luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/602f8d74abdf72db7f5e3ed9_Luos_Logo_animation_Black.gif" alt="Luos logo" title="Luos" align="right" height="60" /></a>

# Robus HAL
> If you want to clone this repo, please refer to the [cloning repository](#cloning-repository) section.

This repository regroups all of the HAL (Hardware Abstraction Layers) routines usable with the Robus network layer
You can choose the one you need for your microcontroller or create your own following our templates and guidelines.
Do not hesitate to share your project and ask us about features on [our Discord](http://bit.ly/JoinLuosDiscord).

# Introduction to Luos :bulb:

We started designing Luos with the conviction that building electronic systems should be made easier than it is today. The majority of development time should be spent on designing the applications and behaviors instead of on complex and time-and-money-eating technicalities. To give a simple example, adding a new sensor (for example a distance sensor) to an electronic device in conception should not take more than a few minutes. So you can try, test and quickly iterate on a project to quickly design what users truly want.

Luos works like a [microservice architecture](https://en.wikipedia.org/wiki/Microservices) in the software world, and a [distributed operating system](https://en.wikipedia.org/wiki/Distributed_operating_system): it encapsulates any software or hardware function to make it communicate and work with any other encapsulated service, however it was developed, either on bare metal or on top of an embedded OS.

This repository contains the Luos library that you will need to include in your software projects. To correctly configure your hardware, it should be used in conjunction with the [LuosHAL project](https://github.com/Luos-io/LuosHAL).

Watch this video for additional details:

<a href="https://www.youtube.com/watch?v=ujh0xNE3TZ8"><img border="0" alt="Luos video" src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/62220e861127837cb8844f56_What%20is%20Luos%20Engine%3F%20Video.png" width="640" height="360"></a>

## Unfamiliar with Luos?

→ You can start by reading the [Basics](https://docs.luos.io/docs/luos-technology/basics/) page.

## You want to tune your device's behavior?

→ You can make your own embedded [Luos apps](https://docs.luos.io/docs/api/list).

→ You can control your devices through a [Gate](https://docs.luos.io/docs/tools/gate) service using [Pyluos](https://docs.luos.io/docs/tools/pyluos).

If you have questions about a specific topic, you can refer or ask it on the [Luos' subreddit](https://www.reddit.com/r/Luos). And if you have suggestions about this documentation, don't hesitate to create pull requests.

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
- STM32G4x1->  LuosHAL default configuration on NUCLEO-G431KB
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


## Don't hesitate to read [our documentation](https://docs.luos.io), or to post your questions/issues on the [Luos' subreddit](https://www.reddit.com/r/Luos/). :books:


[![](https://img.shields.io/reddit/subreddit-subscribers/Luos?style=social)](https://www.reddit.com/r/Luos)
[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://docs.luos.io)
[![](https://img.shields.io/badge/LinkedIn-Follow%20us-0077B5?style=flat&logo=linkedin)](https://www.linkedin.com/company/luos)
