#!/usr/bin/python
# This sript is used to import network dependancies

# Install ws Js dependancy using `npm install ws`
Import("env")

env.Execute("npm install --prefix $BUILD_DIR ws")
