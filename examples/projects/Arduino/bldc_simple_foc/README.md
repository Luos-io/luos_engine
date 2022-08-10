<a href="https://luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/603e0cc45afbb50963aa85f2_Gif%20noir%20rect.gif" alt="Luos logo" title="Luos" align="right" height="100" /></a>

![](https://github.com/Luos-io/luos_engine/actions/workflows/build.yml/badge.svg)
[![](https://img.shields.io/github/license/Luos-io/Luos)](https://github.com/Luos-io/luos_engine/blob/master/LICENSE)

[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://docs.luos.io)
[![](http://certified.luos.io)](https://luos.io)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/luos_engine/library/Luos.svg)](https://registry.platformio.org/libraries/luos_engine/luos_engine)

[![](https://img.shields.io/discord/902486791658041364?label=Discord&logo=discord&style=social)](http://bit.ly/JoinLuosDiscord)
[![](https://img.shields.io/reddit/subreddit-subscribers/Luos?style=social)](https://www.reddit.com/r/Luos)
[![](https://img.shields.io/twitter/url/http/shields.io.svg?style=social)](https://twitter.com/intent/tweet?text=Unleash%20electronic%20devices%20as%20microservices%20thanks%20to%20Luos&https://luos.io&via=Luos_io&hashtags=embeddedsystems,electronics,microservices,api)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)


# simpleFOC stepper project example :bulb:
This project demonstrate how to drive and connect a BLDC motor to a simpleFOC arduino shield, with Arduino Zero. Feel free to use electronics and code example as you want.

## How to compile the code :computer:

 1. Download and install [Platformio](https://platformio.org/platformio-ide)
 2. Open this folder into Platformio
 3. Build (Platformio will do the rest)

## Linked driver
This project is linked to the motor template service.

## Don't hesitate to read [our documentation](https://docs.luos.io), or to post your questions/issues on the [Luos' Forum](https://community.luos.io). :books:

[![](https://img.shields.io/discourse/topics?server=https%3A%2F%2Fcommunity.luos.io&logo=Discourse)](https://community.luos.io)
[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://docs.luos.io)
[![](https://img.shields.io/badge/LinkedIn-Follow%20us-0077B5?style=flat&logo=linkedin)](https://www.linkedin.com/company/luos)

## Configuring Luos network

If you want to connect your mcu into a Luos distributed network, pay attention to the pin Configuration. Due to the use of specific motor drivers, we had to change the standard pinout between the arduino and the breakout boards by the following : 

- PTPA: PIN19 (name A5 on Arduino Zero)
- PTPB: PIN7
- RX_EN: 2
- TX_EN: 3
- TX: PIN_SERIAL1_TX
- RX: PIN_SERIAL1_RX

## How to connect the motor

For this type of motor we need to use an SimpleFOC shield provided by SimpleFOC.

For the wiring of the stepper you should follow the following design:

Add BLDC image here

