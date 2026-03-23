#!/bin/bash
# Inner build script — runs inside the STM32CubeIDE Docker container or locally.
# Usage: _build_ecu.sh TARGET_ECUx [Release|Debug] [--skip-import] [--no-clean] [--no-quiet]
# Set WORKSPACE env var to override the default /workspace path for local builds.

ECU="${1:?Usage: $0 TARGET_ECUx [Release|Debug] [--skip-import] [--no-clean] [--no-quiet]}"
PROJECT_CONF="${2:-Release}"
shift 2 2>/dev/null || shift $#

SKIP_IMPORT=false
BUILD_MODE="-cleanBuild"
EXTRA_DEFINES=""
QUIET=true

while [[ $# -gt 0 ]]; do
	case "$1" in
		--skip-import) SKIP_IMPORT=true; shift ;;
		--no-clean) BUILD_MODE="-build"; shift ;;
		--no-quiet) QUIET=false; shift ;;
		-D) EXTRA_DEFINES="${EXTRA_DEFINES} -D $2"; shift 2 ;;
		-D*) EXTRA_DEFINES="${EXTRA_DEFINES} -D ${1#-D}"; shift ;;
		*) shift ;;
	esac
done

PROJECT_NAME=RAMNV1
WORKSPACE="${WORKSPACE:-/workspace}"
PROJECT_WORKSPACE="${WORKSPACE}/firmware/${PROJECT_NAME}"

set -e
set -o pipefail

if [ "$QUIET" = true ]; then
	QUIET_FILTER='
	BEGIN { dots=0 }
	/Opening/ { if (dots) { printf "\n"; dots=0 }; print; fflush(); next }
	/arm-none-eabi-g/ { last_cc=$0; printf "."; fflush(); dots=1; next }
	/[Ww]arning:| [Ee]rror:|collect2: error:|ld: error:/ {
		if (dots) { printf "\n"; dots=0 }
		if (last_cc != "" && last_cc != shown_cc) { print last_cc; shown_cc=last_cc }
		print; fflush()
		if (/[Ee]rror:|collect2: error:|ld: error:/) exit_code=1;
		next
	}
	/Build Finished/ {
		if (dots) { printf "\n"; dots=0 }; print; fflush();
		if (!/0 errors/) exit_code=1;
		next
	}
	END { if (dots) printf "\n"; if (exit_code) exit exit_code }
	'
fi

BUILD_LOG=$(mktemp)
trap 'rm -f "$BUILD_LOG"' EXIT

run_with_filter() {
	if [ "$QUIET" = true ]; then
		# Use tee to capture everything in BUILD_LOG while awk filters for stdout
		if ! ( "$@" 2>&1 | tee -a "$BUILD_LOG" | awk "$QUIET_FILTER" ); then
			echo "-------------------------------------------------------------------------------"
			echo "Build failed ($1). Full log follows:"
			echo "-------------------------------------------------------------------------------"
			cat "$BUILD_LOG"
			exit 1
		fi
	else
		"$@"
	fi
}

if [ "$SKIP_IMPORT" = false ]; then
	# STM32CubeIDE >= 1.18.0 (docker tag >= 15.0) replaced -import with -importAll
	IMPORT_FLAG="${STM32_IMPORT_FLAG:--import}"
	run_with_filter stm32cubeide --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data /tmp/stm-workspace ${IMPORT_FLAG} ${PROJECT_WORKSPACE}
fi

# Normalize file timestamps to avoid "Clock skew detected" warnings from make
# when the Docker container's clock differs from the host that created the files.
find "${WORKSPACE}" -type f -exec touch -c {} + 2>/dev/null || true

run_with_filter headless-build.sh -data /tmp/stm-workspace ${BUILD_MODE} ${PROJECT_NAME}/${PROJECT_CONF} -D ${ECU} ${EXTRA_DEFINES}
