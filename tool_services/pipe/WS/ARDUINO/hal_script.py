#!/usr/bin/python
import os
from os import path
from os.path import realpath
import click
Import("env")


if (not path.exists("arduinoWebSockets")):
    env.Execute(
        "git clone https://github.com/Links2004/arduinoWebSockets.git")

env.Append(CPPPATH=[realpath("arduinoWebSockets/src")])
env.Append(SRC_FILTER=["+<WS/ARDUINO/arduinoWebSockets/src/WebSockets.cpp>"])
env.Append(SRC_FILTER=["+<WS/ARDUINO/arduinoWebSockets/src/WebSocketsServer.cpp>"])
env.Append(SRC_FILTER=["+<WS/ARDUINO/arduinoWebSockets/src/WebSocketsClient.cpp>"])
env.Append(SRC_FILTER=["+<WS/ARDUINO/arduinoWebSockets/src/SocketIOclient.cpp>"])
click.secho("\t* WebSockets Arduino lib ready.", fg="green")
