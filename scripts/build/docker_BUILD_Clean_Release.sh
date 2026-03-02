#!/bin/bash
# Clean-build all 4 ECUs in Release with the STM32CubeIDE Docker container.

SCRIPT_DIR="$(dirname "${BASH_SOURCE[0]}")"

if [ -e /workspace ]; then
	set -e
	OUTPUT_FOLDER="/workspace/scripts/firmware"
	PROJECT_NAME=RAMNV1
	mkdir -p "${OUTPUT_FOLDER}"
	bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUA Release
	cp -pr "/workspace/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUA.hex"
	bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUB Release --skip-import
	cp -pr "/workspace/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUB.hex"
	bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUC Release --skip-import
	cp -pr "/workspace/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUC.hex"
	bash "${SCRIPT_DIR}/_build_ecu.sh" TARGET_ECUD Release --skip-import
	cp -pr "/workspace/firmware/${PROJECT_NAME}/Release/${PROJECT_NAME}.hex" "${OUTPUT_FOLDER}/ECUD.hex"
	exit 0
fi

STM32CUBEIDEWORKSPACE="$( cd "${SCRIPT_DIR}/../../" ; pwd )"
source "${SCRIPT_DIR}/_version.sh"

( cd "${STM32CUBEIDEWORKSPACE}"; docker run --rm -e STM32_IMPORT_FLAG -v .:/workspace xanderhendriks/stm32cubeide:${DOCKER_STM32CUBEIDE_TAG} /workspace/scripts/build/docker_BUILD_Clean_Release.sh ) || exit 1
