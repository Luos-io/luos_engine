# LUOS Engine sample running on Zephyr RTOS


## Overview

This sample demonstrates how to setup, build and 
flash a simple LED service to demonstrate the Luos Engine
on top of the Zephyr RTOS.

## Requirements

You need to install Zephyr dependencies first before using this sample please
check here how to do that: https://docs.zephyrproject.org/latest/develop/getting_started/index.html


## Setup, Building and Running

First of all use west Zephyr meta tool to pull the Luos Engine as zephyr 
module, this will setup the workspace and also will pull the Zephyr kernel
along it:

```console
$ west init -m git@github.com:Luos-io/luos_engine.git
$ cd luos_engine
$ west update
```
Now you should be able to compile the example app.

To run this example on the **STM Microelectronics Nucleo G474RE** board, run the
following commands which will compile the application, run it on the emulator,
and output the result to the console:

```console
$ west build -p auto -b nucleo_g474re path/to/luos_engine/examples/projects/zephyr
```
To flash the firmware: 

```console
$ west flash
```