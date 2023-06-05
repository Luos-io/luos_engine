#!/usr/bin/env python
import os
os.system("lcov -d .pio/build/native/ -c -o lcov.info")
os.system("lcov --remove lcov.info '*/Platforms/*' '*/bootloader/*' '*/.pio/*' '*/HAL/*' '*/test/*' -o filtered_lcov.info")
os.system("genhtml filtered_lcov.info -o cov/ --demangle-cpp")
