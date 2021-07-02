RAMN_PORT=/dev/ttyACM0

ECU_FIRMWARE_PATH=../../firmware

python3 ECUA_goToDFU.py $RAMN_PORT

sleep 2

dfu-util  -d 0x0483:0xdf11 -c1 -a0 -D "$ECU_FIRMWARE_PATH/ECUA.hex"

#dfu-util --dfuse-address -d 0483:df11 -c 1 -i 0 -a 0 -D $ECU_FIRMWARE_PATH/ECUA.hex



sleep 2
