RAMN_PORT=AUTO

ECU_FIRMWARE_PATH=../../firmware

python3 ../canboot.py $RAMN_PORT B -i $ECU_FIRMWARE_PATH/ECUB.hex -e -p -v

python3 ../canboot.py $RAMN_PORT C -i $ECU_FIRMWARE_PATH/ECUC.hex -e -p -v

python3 ../canboot.py $RAMN_PORT D -i $ECU_FIRMWARE_PATH/ECUD.hex -e -p -v --reset
