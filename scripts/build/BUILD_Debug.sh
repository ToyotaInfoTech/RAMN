#!/bin/bash
# Incremental-build all 4 ECUs in Debug with a locally installed STM32CubeIDE.
# Requires stm32cubeide and headless-build.sh in PATH.

SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"
export WORKSPACE="$( cd "${SCRIPT_DIR}/../../" ; pwd )"
OUTPUT_FOLDER="${SCRIPT_DIR}/../firmware"
PROJECT_NAME=RAMNV1

set -e

mkdir -p "${OUTPUT_FOLDER}"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUA Debug --no-clean
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Debug/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUA.hex"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUB Debug --skip-import --no-clean
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Debug/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUB.hex"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUC Debug --skip-import --no-clean
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Debug/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUC.hex"
bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUD Debug --skip-import --no-clean
cp -pr "${WORKSPACE}/firmware/${PROJECT_NAME}/Debug/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUD.hex"
