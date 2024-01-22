<a href="https://luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/603e0cc45afbb50963aa85f2_Gif%20noir%20rect.gif" alt="Luos logo" title="Luos" align="right" height="100" /></a>

![](https://github.com/Luos-io/luos_engine/actions/workflows/build.yml/badge.svg)
[![](https://img.shields.io/github/license/Luos-io/Luos)](https://github.com/Luos-io/luos_engine/blob/master/LICENSE)

[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://www.luos.io/docs/)
[![](http://certified.luos.io)](https://luos.io)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/luos/library/luos_engine.svg)](https://registry.platformio.org/libraries/luos_engine/luos_engine)

[![](https://img.shields.io/discord/902486791658041364?label=Discord&logo=discord&style=social)](http://bit.ly/JoinLuosDiscord)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)

# The Luos ping pong world cup project example :bulb:

```
The Luos ping pong world cup!
                          ((((.
                     ,(((((((((((((
          (#########(((((((((((((((((
        ###########(((((((((((((((((((
      ,###########((((((((((((((((((((
      #############(((((((((((((((((((
      ,,*###########(((((((((((((((((
       ,,,,###########,,,,,,,,,,,,.
      ***,,,,*#########  ,*****
   *******  .,,,###       ****.
 ******            *//    ****,
  ***             *////   *****
                    ''      .
Controls:
     - If the ball is going left press '←'
     - If the ball is going right press '→'
    Press SPACE BAR to start!
```

This project demonstrate how to make and use a simple multiplayer game through Luos.
This small and simple game run on your computer terminal and allow you to play with your friends.

## Before playing :computer:

1.  Open a terminal on this project and run the broker at a given IP and port `./../broker.py --ip 'YOUR_LOCAL_IP' -p 8000`

You only need to have one somewhere running, it's a kind of server, then anyone on your local network will be able to play.

## How to compile and play the game :video_game:

1.  Install GCC on your computer
2.  Download and install [Platformio](https://platformio.org/platformio-ide)
3.  Open this folder into Platformio
4.  Set the broker IP and port on the `node_config.h` file by replacing `#define WS_BROKER_ADDR "ws://127.0.0.1:8000"` with the corect IP and port
5.  Build (Platformio will do the rest)
6.  Open a new terminal on this projet and run the compiled binary `./.pio/build/native/program`

## Don't hesitate to read [our documentation](https://www.luos.io/docs/), or to post your questions/issues on the [Luos' Forum](https://community.luos.io). :books:

[![](https://img.shields.io/discourse/topics?server=https%3A%2F%2Fcommunity.luos.io&logo=Discourse)](https://community.luos.io)
[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://www.luos.io/docs/)
[![](https://img.shields.io/badge/LinkedIn-Follow%20us-0077B5?style=flat&logo=linkedin)](https://www.linkedin.com/company/luos)
