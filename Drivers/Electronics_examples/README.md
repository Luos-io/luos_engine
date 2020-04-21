<a href="https://luos.io"><img src="https://www.luos.io/wp-content/uploads/2020/03/Luos-color.png" alt="Luos logo" title="Luos" align="right" height="60" /></a>

[![](http://certified.luos.io)](https://luos.io)
[![](https://img.shields.io/github/license/Luos-io/Electronics_examples)](https://github.com/Luos-io/Electronics_examples/blob/master/LICENSE)

# Electronics examples

This repository contains hardwares librairies used by Luos and the Luos developpement Board to inspire you to create your own designs :
 - **L0** : The base board of all Luos [prototyping boards](https://doc.luos.io/_pages/prototyping_boards/boards-list.html)
 - **STm32f0_disco_luos_shield** a debugging shield equivalent to an L0 for STM32F072B-DISCO dev board
 - **Breakout_board** : a small add on board to make any board compatible with Luos.

The Luos librairies allow you to open all the examples shields of Luos.

Feel free to produce and use all our design to create your own.

## How to start

1) install KiCad: https://www.kicad-pcb.org/

2) Once Kicad is download open it and setup libraries
    There is 3 libs :

    - **Schematic**: You must go on "Preferences" -> "Manage Symbol Librairies". Click on the (+) button and type: Common_Lib in Nickname field  and type the path of the Common_Lib.lib file in Library Path field (Electronics/00_Common_Libraries/Common_Library/Common_Lib.lib).

    - **PCB**: You must go on "Preferences" -> "Manage Footprint Librairies". Click on the (+) button and type: Common_Footprint in Nickname field  and type the path of the Common_Footprint.pretty file in Library Path field(Electronics/00_Common_Libraries/Common_Footprint.pretty).

    - **3D body**: You must go on "Preferences" -> "Configure Paths". On the KISYS3DMOD line type the path of the Common_Mecanic Folder(Electronics\00_Common_Libraries\Common_Mecanic).



    
3) click on File -> Open project

4) Go to the KiCad project folder you want open and click on the .pro file.


[![](https://img.shields.io/discourse/topics?server=https%3A%2F%2Fcommunity.luos.io&logo=Discourse)](https://community.luos.io)
[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://doc.luos.io)
[![](https://img.shields.io/badge/LinkedIn-Follow%20us-0077B5?style=flat&logo=linkedin)](https://www.linkedin.com/company/luos)
