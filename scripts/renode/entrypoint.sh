#!/usr/bin/env bash
# Copyright (c) 2024 TOYOTA MOTOR CORPORATION. ALL RIGHTS RESERVED.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

set -euo pipefail

export PATH="/opt/renode:${PATH}"

RESC_FILE="${1:-/opt/renode/ramn_4ecu.resc}"

if [ ! -f "${RESC_FILE}" ]; then
    echo "[-] Error: Simulation script ${RESC_FILE} not found." >&2
    exit 1
fi

export RESC_FILE="${RESC_FILE}"
exec python3 /opt/renode/simulator_proxy.py
