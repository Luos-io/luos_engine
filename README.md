<a href="https://luos.io"><img src="https://uploads-ssl.webflow.com/601a78a2b5d030260a40b7ad/602f8d74abdf72db7f5e3ed9_Luos_Logo_animation_Black.gif" alt="Luos logo" title="Luos" align="right" height="60" /></a>

[![](http://certified.luos.io)](https://luos.io)
[![](https://img.shields.io/twitter/url/http/shields.io.svg?style=social)](https://twitter.com/intent/tweet?text=Unleash%20electronic%20devices%20as%20microservices%20thanks%20to%20Luos&https://luos.io&via=Luos_io&hashtags=embeddedsystems,electronics,microservices,api)
[![](https://img.shields.io/badge/LinkedIn-Share-0077B5?style=social&logo=linkedin)](https://www.linkedin.com/sharing/share-offsite/?url=https%3A%2F%2Fgithub.com%2Fluos-io)

> If you want to clone this repo, please refer to [cloning repository](#cloning-repository) section.

# Introduction to Luos? :bulb:

We started designing Luos with the conviction that building electronic systems should be made easier than it is today. The majority of development time should be spent on designing the applications and behaviors instead of on complex and time-and-money-eating technicalities. To give a simple example, adding a new sensor (for example a distance sensor) to an electronic device in conception should not take more than a few minutes. So you can try, test and quickly iterate on a project to quickly design what users truly want.

Luos works like a [microservice architecture](https://en.wikipedia.org/wiki/Microservices) in the software world, and a [distributed operating system](https://en.wikipedia.org/wiki/Distributed_operating_system): it encapsulates any software or hardware function to make it communicate and work with any other encapsulated module, however it was developed, either on bare metal or on top of an embedded OS.

This repository contains the Luos library that you will need to include in your software projects. To correctly configure your hardware, iIt should be used in conjunction with the [LuosHAL project](https://github.com/Luos-io/LuosHAL).

Watch this video for additional details:

<a href="https://youtu.be/xQe3z0M_FE8"><img border="0" alt="Luos video" src="https://www.luos.io/wp-content/uploads/2019/11/youtube.jpeg" width="640" height="360"></a>

## Unfamiliar with Luos?

→ You can start by reading the [Basics](https://docs.luos.io/pages/overview/general-basics.html) page.

## You want to make your own board with [Luos modules](https://docs.luos.io/pages/low/modules/create-modules.html)?

→ Start reading how to integrate Luos in your [Development environment](https://docs.luos.io/pages/low/dev-env.html).

→ Then learn how to [Create a Luos project](https://docs.luos.io/pages/low/modules/create-project.html).

## You want to tune your device's behavior?

→ Luos provides a sets of [Prototyping boards](https://docs.luos.io/pages/prototyping_boards/boards-list.html) you can use as example or to develop your own project.

→ You can make your own embedded [Luos apps](https://docs.luos.io/pages/low/modules/create-modules.html).

→ You can control your devices through a [Gate](https://docs.luos.io/pages/high/modules_list/gate.html) module using [Pyluos](https://docs.luos.io/pages/high/pyluos.html).

If you have questions about a specific topic, you can refer or ask it on the [Luos' Forum](https://community.luos.io/). And if you have suggestions about this documentation, don't hesitate to create pull requests.

## Don't hesitate to read [our documentation](https://docs.luos.io), or to post your questions/issues on the [Luos' Forum](https://community.luos.io). :books:

# Cloning repository

This repository contains a submodule. The `examples` folder is linked to another repository. That means this folder won't be cloned with a regular `git clone`.

If you want to clone the `examples` folder along with this repo, type:

```
git clone --recursive https://github.com/Luos-io/Luos.git
```

If you already cloned this repository and want to update the submodule `examples`, type:

```
git submodule update --init --recursive
```

[![](https://img.shields.io/discourse/topics?server=https%3A%2F%2Fcommunity.luos.io&logo=Discourse)](https://community.luos.io)
[![](https://img.shields.io/badge/Luos-Documentation-34A3B4)](https://docs.luos.io)
[![](https://img.shields.io/badge/LinkedIn-Follow%20us-0077B5?style=flat&logo=linkedin)](https://www.linkedin.com/company/luos)
