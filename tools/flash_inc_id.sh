#!/bin/bash

#HEX="C:\Users\kiu\Documents\projects\hardware\done\xcb-2024\firmware\XCB-2024-revb\Debug\XCB-2024-revb.hex"
#HEX="C:\Users\kiu\Documents\projects\hardware\done\xcb-2024\firmware\XCB-2024-revb\Release\XCB-2024-revb.hex"

HEX="C:\Users\kiu\Documents\projects\hardware\done\xcb-2024\tools\xcb-2024-revb-F01.hex"

ID=01

if [ $# -eq 1 ]; then
  ID=$(echo "$1" | tr a-z A-Z)
fi

dd if=/dev/zero of=./tmp.bin count=2048 bs=1 status=none > /dev/null

while true
do
  if [ ${#ID} -eq 1 ]
  then
    ID=0${ID}
  fi

  clear
  echo "--- Flashing ID: ${ID} ---"
  echo

  echo -n -e "\x42\x${ID}" | dd of=./tmp.bin bs=1 seek=0 count=2 conv=notrunc status=none > /dev/null
  echo -n -e "\x42\x${ID}" | dd of=./tmp.bin bs=1 seek=1024 count=2 conv=notrunc status=none > /dev/null

  /mnt/c/Program\ Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe -c port=SWD reset=HWrst -e all -w ${HEX} -v -w tmp.bin 0x08007800 -hardRst
  #-e 30 -w32 0x08007800 0x0000${ID}42
  #-w8 0x08007800 0x42 0x${ID}
  #-r8 0x08007800 4

  if [ $? -eq 0 ]; then
    echo
    echo "--- Done ID: ${ID} ---"
    read cont
    ID=$(echo "obase=ibase=16;${ID}+1" | bc)
  else
    echo
    echo "*** FAIL ID: ${ID} ***"
    read cont
  fi

done
