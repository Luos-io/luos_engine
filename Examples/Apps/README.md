<a href="https://luos.io"><img src="https://www.luos.io/wp-content/uploads/2020/03/Luos-color.png" alt="Luos logo" title="Luos" align="right" height="60" /></a>

[![](http://certified.luos.io)](https://luos.io)
[![](https://img.shields.io/github/license/Luos-io/Examples)](
https://github.com/Luos-io/Examples/blob/master/LICENSE)

[![](https://img.shields.io/twitter/url/http/shields.io.svg?style=social)](https://twitter.com/intent/tweet?text=Unleash%20electronic%20devices%20as%20microservices%20thanks%20to%20Luos&https://luos.io&via=Luos_io&hashtags=embeddedsystems,electronics,microservices,api)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)

# Apps

This folder contains the service's sources of each **app** project (see [Apps section](https://docs.luos.io/pages/low/services/create-services.html?#apps-guidelines) in documentation). There are three folders:

 - **Alarm_controller** - Part of the bike alarm App. This App communicates with an accelerometer service, and monitors for movement. If movement is detected, it will look for a Service to issue an alarm.
 - **Gate** - A special App used in conjunction with the [Pyluos](https://github.com/Luos-io/Pyluos) library, this App allows Python to taeke control of the Luos network, and to communicate with the different Services located on the network.
 - **Start_controller** - Part of the bike alarm App. This App looks for the bike lock. If the lock has been activated, then it will inform the Alarm Controller, and set detection in motion.


[![](https://img.shields.io/discourse/topics?server=https%3A%2F%2Fcommunity.luos.io&logo=Discourse)](https://community.luos.io)
[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://docs.luos.io)
[![](https://img.shields.io/badge/LinkedIn-Follow%20us-0077B5?style=flat&logo=linkedin)](https://www.linkedin.com/company/luos)