#!/bin/bash
# Clean-build all 4 ECUs in Release with a locally installed STM32CubeIDE.
# Requires stm32cubeide and headless-build.sh in PATH.

SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
export WORKSPACE="$( cd "${SCRIPT_DIR}/../../" ; pwd )"
OUTPUT_FOLDER="${SCRIPT_DIR}/../firmware"
PROJECT_NAME=RAMNV1

set -e

mkdir -p "${OUTPUT_FOLDER}"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUA Release
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUA.hex"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUB Release --skip-import
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUB.hex"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUC Release --skip-import
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUC.hex"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUD Release --skip-import
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUD.hex"
