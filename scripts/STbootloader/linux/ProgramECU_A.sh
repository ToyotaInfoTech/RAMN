ECU_FIRMWARE_PATH=../../firmware

dfu-util  -d 0x0483:0xdf11 -c1 -a0 -D "$ECU_FIRMWARE_PATH/ECUA.bin" --dfuse-address 0x08000000:leave	 


sleep 10
