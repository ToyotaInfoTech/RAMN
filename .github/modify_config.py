#!/usr/bin/env python3
"""Modify ramn_config.h to enable/disable macros for variant builds.

Usage:
    python3 modify_config.py --enable "MACRO1 MACRO2" --disable "MACRO3" \
        --ecu TARGET_ECUA --variant gsusb

Writes a step summary table to $GITHUB_STEP_SUMMARY when available.
"""

import argparse
import os
import re
import sys


def main():
    parser = argparse.ArgumentParser(description="Modify ramn_config.h macros")
    parser.add_argument("--enable", default="", help="Space-separated macros to enable")
    parser.add_argument(
        "--disable", default="", help="Space-separated macros to disable"
    )
    parser.add_argument(
        "--config", default="firmware/RAMNV1/Core/Inc/ramn_config.h", help="Config path"
    )
    parser.add_argument("--ecu", default="", help="ECU target name")
    parser.add_argument("--variant", default="", help="Variant name")
    args = parser.parse_args()

    enable_macros = args.enable.split() if args.enable.strip() else []
    disable_macros = args.disable.split() if args.disable.strip() else []

    with open(args.config) as f:
        content = f.read()

    # Disable macros: comment out #define lines with word-boundary matching
    for macro in disable_macros:
        content = re.sub(
            rf"^(\s*)#define {macro}([^A-Za-z0-9_]|$)",
            rf"\1//#define {macro}\2",
            content,
            flags=re.MULTILINE,
        )

    # When GENERATE_RUNTIME_STATS is disabled, also patch FreeRTOSConfig.h:
    # - Set configGENERATE_RUN_TIME_STATS to 0
    # - Comment out portCONFIGURE_TIMER_FOR_RUN_TIME_STATS (called
    #   unconditionally in tasks.c, so it must resolve or be undefined)
    # - Comment out portGET_RUN_TIME_COUNTER_VALUE
    if "GENERATE_RUNTIME_STATS" in disable_macros:
        frc_path = os.path.join(os.path.dirname(args.config), "FreeRTOSConfig.h")
        with open(frc_path) as f:
            frc = f.read()
        frc = re.sub(
            r"#define configGENERATE_RUN_TIME_STATS\s+1",
            "#define configGENERATE_RUN_TIME_STATS 0",
            frc,
        )
        frc = re.sub(
            r"^#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS",
            "// #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS",
            frc,
            flags=re.MULTILINE,
        )
        frc = re.sub(
            r"^#define portGET_RUN_TIME_COUNTER_VALUE",
            "// #define portGET_RUN_TIME_COUNTER_VALUE",
            frc,
            flags=re.MULTILINE,
        )
        with open(frc_path, "w") as f:
            f.write(frc)

    # Enable macros: uncomment //#define or // #define lines
    for macro in enable_macros:
        content = re.sub(
            rf"^(\s*)//\s*#define {macro}([^A-Za-z0-9_]|$)",
            rf"\1#define {macro}\2",
            content,
            flags=re.MULTILINE,
        )

    with open(args.config, "w") as f:
        f.write(content)

    # Write step summary
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY", "")
    if summary_path:
        with open(summary_path, "a") as f:
            f.write(f"## Macro Coverage Build: `{args.ecu}` / `{args.variant}`\n\n")
            f.write("| Action | Macro |\n")
            f.write("|--------|-------|\n")
            for macro in disable_macros:
                f.write(f"| Disabled | `{macro}` |\n")
            if "GENERATE_RUNTIME_STATS" in disable_macros:
                f.write("| Disabled | `configGENERATE_RUN_TIME_STATS` (FreeRTOS) |\n")
            for macro in enable_macros:
                f.write(f"| Enabled | `{macro}` |\n")
            f.write("\n")


if __name__ == "__main__":
    main()
