<a href="https://luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/603e0cc45afbb50963aa85f2_Gif%20noir%20rect.gif" alt="Luos logo" title="Luos" align="right" height="100" /></a>

![](https://github.com/Luos-io/luos_engine/actions/workflows/build.yml/badge.svg)
[![](https://img.shields.io/github/license/Luos-io/Luos)](https://github.com/Luos-io/luos_engine/blob/master/LICENSE)

[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://www.luos.io)
[![](http://certified.luos.io)](https://luos.io)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/luos_engine/library/Luos.svg)](https://registry.platformio.org/libraries/luos_engine/luos_engine)

[![](https://img.shields.io/discord/902486791658041364?label=Discord&logo=discord&style=social)](http://bit.ly/JoinLuosDiscord)
[![](https://img.shields.io/reddit/subreddit-subscribers/Luos?style=social)](https://www.reddit.com/r/Luos)
[![](https://img.shields.io/twitter/url/http/shields.io.svg?style=social)](https://twitter.com/intent/tweet?text=Unleash%20electronic%20devices%20as%20microservices%20thanks%20to%20Luos&https://luos.io&via=Luos_io&hashtags=embeddedsystems,electronics,microservices,api)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)

# Apps

This folder contains the service's sources of each **app** project (see [Apps section](https://www.luos.io/docs/next/luos-technology/services/service-api#apps-guidelines) in documentation). There are three folders:

 - **Alarm_controller** - Part of the bike alarm App. This App communicates with an accelerometer service, and monitors for movement. If movement is detected, it will look for a Service to issue an alarm.
 - **Gate** - A special App used in conjunction with the [Pyluos](https://github.com/Luos-io/Pyluos) library, this App allows Python to taeke control of the Luos network, and to communicate with the different Services located on the network.
 - **Start_controller** - Part of the bike alarm App. This App looks for the bike lock. If the lock has been activated, then it will inform the Alarm Controller, and set detection in motion.
