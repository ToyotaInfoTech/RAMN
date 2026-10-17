#!/usr/bin/env bash
# Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

FIRMWARE_DIR="${FIRMWARE_DIR:-${REPO_ROOT}/scripts/firmware}"
RENODE_PORT="${RENODE_PORT:-29536}"
IMAGE_NAME="ramn-renode-sim"
CONTAINER_NAME="ramn-renode-sim-ci-${RANDOM}"

# Validate required firmware binaries exist
for ecu in ECUA ECUB ECUC ECUD; do
    if [ ! -f "${FIRMWARE_DIR}/${ecu}.hex" ]; then
        echo "[-] Error: ${FIRMWARE_DIR}/${ecu}.hex not found. Build firmware before running runtime simulation tests." >&2
        exit 1
    fi
done

echo "[+] Building Renode simulation Docker container..."
docker build -t "${IMAGE_NAME}" "${REPO_ROOT}/scripts/renode/"

# Clean up any previous container with same name
docker rm -f "${CONTAINER_NAME}" 2>/dev/null || true

# Register trap to clean up simulator container on exit or failure
trap 'status=$?; if [ $status -ne 0 ]; then echo "[-] Container logs on failure:"; docker logs "${CONTAINER_NAME}"; fi; echo "[+] Cleaning up Renode container ${CONTAINER_NAME}..."; docker rm -f "${CONTAINER_NAME}" >/dev/null 2>&1 || true' EXIT

echo "[+] Starting RAMN 4-ECU simulation in Renode Docker container..."
docker run -d --name "${CONTAINER_NAME}" -p "${RENODE_PORT}:29536" \
    -v "${FIRMWARE_DIR}:/firmware:ro" \
    -v "${REPO_ROOT}/scripts/renode/ramn_4ecu.resc:/opt/renode/ramn_4ecu.resc:ro" \
    "${IMAGE_NAME}"

echo "[+] Waiting for Renode simulation and superserver proxy readiness..."
start_time=$(date +%s)
ready=0
while [ $(($(date +%s) - start_time)) -lt 90 ]; do
    if python3 -c "import socket; s = socket.socket(); s.settimeout(1.5); s.connect(('127.0.0.1', ${RENODE_PORT})); s.sendall(b'O\r'); resp = s.recv(16); s.close(); exit(0 if (b'\r' in resp or b'\n' in resp) else 1)" 2>/dev/null; then
        ready=1
        break
    fi
    sleep 0.5
done
if [ "${ready}" -ne 1 ]; then
    echo "[-] Error: Simulator port ${RENODE_PORT} failed to become ready within 90s" >&2
    docker logs "${CONTAINER_NAME}"
    exit 1
fi


echo "[+] Executing Renode runtime test suite..."
export RENODE_SIM_REQUIRED="1"
export RENODE_HOST="127.0.0.1"
export RENODE_PORT="${RENODE_PORT}"
PYTHONPATH="${REPO_ROOT}/scripts/tests:${PYTHONPATH:-}" python3 -m pytest -v "${REPO_ROOT}/scripts/tests/test_renode_runtime.py"

echo "[+] Renode runtime simulation tests PASSED successfully."
