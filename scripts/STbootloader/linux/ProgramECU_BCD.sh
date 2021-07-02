RAMN_PORT=/dev/ttyACM0

ECU_FIRMWARE_PATH=../../firmware

python3 canboot.py $RAMN_PORT B -i $ECU_FIRMWARE_PATH/ECUB.hex -e -p -v -ru -wu

python3 canboot.py $RAMN_PORT C -i $ECU_FIRMWARE_PATH/ECUC.hex -e -p -v -ru -wu

python3 canboot.py $RAMN_PORT D -i $ECU_FIRMWARE_PATH/ECUD.hex -e -p -v -ru -wu --reset