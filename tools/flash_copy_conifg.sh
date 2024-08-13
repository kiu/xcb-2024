#!/bin/bash

clear
echo "--- Reading Configuration ---"
echo

/mnt/c/Program\ Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe -c port=SWD reset=HWrst -u 0x08007800 2048 tmp.bin -hardRst

hexdump tmp.bin

echo "--- Press enter to write configuration ---"
read

clear
echo "--- Writing Configuration ---"
echo

/mnt/c/Program\ Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe -c port=SWD reset=HWrst -d tmp.bin 0x08007800 -hardRst
