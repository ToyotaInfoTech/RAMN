#!/usr/bin/env python3
"""Generate the Build & Macro Coverage Report.

Reads build-metrics and macro-results artifacts, scans ramn_config.h for
ENABLE_* macros, and produces a Markdown report (report.md).

Usage:
    python3 generate_report.py \
        --metrics-dir all-build-metrics \
        --results-dir all-macro-results \
        --config firmware/RAMNV1/Core/Inc/ramn_config.h \
        --total-lines N --compiled-lines N --coverage-pct N
"""

import argparse
import os
import re
import sys

# ---------------------------------------------------------------------------
# Hex size table
# ---------------------------------------------------------------------------


def parse_hex_sizes(metrics_dir):
    """Return {(conf, tag): {ecu: size_str}} from build-metrics artifacts."""
    sizes = {}
    if not os.path.isdir(metrics_dir):
        return sizes
    for entry in sorted(os.listdir(metrics_dir)):
        d = os.path.join(metrics_dir, entry)
        txt = os.path.join(d, "hex-sizes.txt")
        if not os.path.isfile(txt):
            continue
        # directory name like build-metrics-RAMNV1-Release-15.0
        if not entry.startswith("build-metrics-"):
            continue
        parts = entry.split("-")
        if len(parts) < 5:
            continue
        tag = parts[-1]
        conf = parts[-2]
        key = (conf, tag)
        sizes.setdefault(key, {})
        with open(txt) as f:
            for line in f:
                line = line.strip()
                if "=" not in line:
                    continue
                ecu, size = line.split("=", 1)
                sizes[key][ecu] = size
    return sizes


def format_size(raw):
    """Format a raw byte string as 'N KiB (N bytes)' or 'N/A'."""
    if not raw or raw == "N/A":
        return "N/A"
    try:
        n = int(raw)
        return f"{n // 1024} KiB ({n} bytes)"
    except ValueError:
        return raw


def hex_size_table(sizes):
    """Return Markdown table rows for hex sizes, grouped by docker tag."""
    configs = sorted(sizes.keys())
    if not configs:
        return "| ECU | Release | Debug |\n|-----|---------|-------|\n"

    headers = ["ECU"] + [f"{conf} (tag {tag})" for conf, tag in configs]
    lines = ["| " + " | ".join(headers) + " |"]
    lines.append("|" + "|".join(["-----"] * len(headers)) + "|")
    for ecu in ("ECUA", "ECUB", "ECUC", "ECUD"):
        row = [ecu]
        for key in configs:
            row.append(format_size(sizes[key].get(ecu, "N/A")))
        lines.append("| " + " | ".join(row) + " |")
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Macro coverage results table
# ---------------------------------------------------------------------------


def parse_macro_results(results_dir):
    """Return list of dicts from macro-results artifacts."""
    results = []
    if not os.path.isdir(results_dir):
        return results
    for entry in sorted(os.listdir(results_dir)):
        txt = os.path.join(results_dir, entry, "result.txt")
        if not os.path.isfile(txt):
            continue
        rec = {}
        with open(txt) as f:
            for line in f:
                line = line.strip()
                if "=" not in line:
                    continue
                key, value = line.split("=", 1)
                rec[key] = value
        results.append(rec)
    return results


def macro_results_table(results):
    """Return Markdown table rows for macro coverage results."""
    lines = []
    lines.append("| ECU | Variant | Macros Changed | Result | Hex Size | Warnings |")
    lines.append("|-----|---------|----------------|--------|----------|----------|")
    for rec in results:
        ecu = rec.get("ecu", "")
        variant = rec.get("variant", "")
        enable = rec.get("enable", "")
        disable = rec.get("disable", "")
        outcome = rec.get("outcome", "")
        hex_size = rec.get("hex_size", "N/A")
        warnings = rec.get("warnings", "0")

        mc_parts = []
        for m in enable.split():
            if m:
                mc_parts.append(f"+`{m}`")
        for m in disable.split():
            if m:
                mc_parts.append(f"\u2212`{m}`")
        mc = " ".join(mc_parts)

        res = "\u2705 Pass" if outcome == "success" else "\u274c Fail"

        if hex_size and hex_size != "N/A":
            try:
                hf = f"{int(hex_size) // 1024} KiB"
            except ValueError:
                hf = hex_size
        else:
            hf = "N/A"

        wf = warnings if warnings else "0"

        ecu_short = ecu.replace("TARGET_", "")
        lines.append(f"| `{ecu_short}` | {variant} | {mc} | {res} | {hf} | {wf} |")
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Dynamic ENABLE_ macro coverage table
# ---------------------------------------------------------------------------


def scan_macros(config_path):
    """Scan ramn_config.h for all ENABLE_*, WATCHDOG_ENABLE,
    GENERATE_RUNTIME_STATS macros and their default state per ECU target.

    Returns a list of (macro_name, {target: enabled_bool}).
    """
    with open(config_path) as f:
        lines = f.readlines()

    # Macros we care about
    interesting = re.compile(r"^(ENABLE_\w+|WATCHDOG_ENABLE|GENERATE_RUNTIME_STATS)$")

    # Track which target section we're in
    current_target = None
    depth = 0
    target_depth = 0
    guard_depth = 0
    targets = ["TARGET_ECUA", "TARGET_ECUB", "TARGET_ECUC", "TARGET_ECUD"]

    # {macro: {target: True/False}}
    macro_states = {}
    # Track macros in common (outside target blocks) and per-target
    common_macros = {}  # macro -> True/False (enabled/commented)
    target_macros = {t: {} for t in targets}

    for line in lines:
        s = line.strip()

        # Track preprocessor depth
        if re.match(r"#\s*(?:ifdef|ifndef|if)\b", s):
            depth += 1
            if depth == 1 and re.match(r"#\s*ifndef\s+INC_", s):
                guard_depth = 1
            elif current_target is None:
                for t in targets:
                    if t in s and "defined" in s and "!" not in s:
                        current_target = t
                        target_depth = depth
                        break
        elif s.startswith("#endif"):
            if current_target and depth == target_depth:
                current_target = None
            depth = max(depth - 1, 0)

        # Check for #define or //#define
        m_active = re.match(r"^\s*#define\s+(\w+)", s)
        m_commented = re.match(r"^\s*//\s*#define\s+(\w+)", s)

        macro = None
        enabled = False
        if m_active:
            macro = m_active.group(1)
            enabled = True
        elif m_commented:
            macro = m_commented.group(1)
            enabled = False

        if macro and interesting.match(macro):
            if macro.startswith("INC_"):
                continue
            if current_target:
                target_macros[current_target][macro] = enabled
            elif depth == guard_depth:
                common_macros[macro] = enabled

    # Build unified view: for each macro, determine state per target
    all_macros = set(common_macros.keys())
    for t in targets:
        all_macros |= set(target_macros[t].keys())

    result = []
    for macro in sorted(all_macros):
        states = {}
        for t in targets:
            if macro in target_macros[t]:
                states[t] = target_macros[t][macro]
            elif macro in common_macros:
                states[t] = common_macros[macro]
            else:
                states[t] = False
        result.append((macro, states))
    return result


def build_macro_coverage_table(macros, macro_results):
    """Build the ENABLE_ macro coverage table dynamically.

    For each macro, determine:
    - Which ECUs have it ON/OFF by default
    - Which variants enable/disable it
    - Whether it's covered (tested in both ON and OFF states)
    """
    # Parse variants from macro_results
    variants = []
    for rec in macro_results:
        ecu = rec.get("ecu", "")
        variant = rec.get("variant", "")
        enable = set(rec.get("enable", "").split()) - {""}
        disable = set(rec.get("disable", "").split()) - {""}
        variants.append((ecu, variant, enable, disable))

    lines = []
    lines.append("| # | Macro | Tested ON | Tested OFF | Covered |")
    lines.append("|---|-------|-----------|------------|---------|")

    targets = ["TARGET_ECUA", "TARGET_ECUB", "TARGET_ECUC", "TARGET_ECUD"]
    target_short = {t: t.replace("TARGET_", "") for t in targets}

    for i, (macro, states) in enumerate(macros, 1):
        # Default ON/OFF targets
        default_on = [t for t in targets if states.get(t, False)]
        default_off = [t for t in targets if not states.get(t, False)]

        # Variants that enable/disable this macro
        variant_on = []
        variant_off = []
        for ecu, variant, enable, disable in variants:
            if macro in enable:
                variant_on.append(f"variant: {variant} ({target_short.get(ecu, ecu)})")
            if macro in disable:
                variant_off.append(f"variant: {variant} ({target_short.get(ecu, ecu)})")

        # Build "Tested ON" description
        on_parts = []
        if default_on:
            on_parts.append(
                "default (" + ", ".join(target_short[t] for t in default_on) + ")"
            )
        on_parts.extend(variant_on)
        tested_on = ", ".join(on_parts) if on_parts else "\u2014"

        # Build "Tested OFF" description
        off_parts = []
        if default_off:
            off_parts.append(
                "default (" + ", ".join(target_short[t] for t in default_off) + ")"
            )
        off_parts.extend(variant_off)
        tested_off = ", ".join(off_parts) if off_parts else "\u2014"

        # Covered if tested in both ON and OFF states
        has_on = bool(default_on or variant_on)
        has_off = bool(default_off or variant_off)
        covered = "\u2705" if (has_on and has_off) else "\u2b1c"

        lines.append(f"| {i} | `{macro}` | {tested_on} | {tested_off} | {covered} |")

    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(description="Generate build coverage report")
    parser.add_argument(
        "--metrics-dir", default="all-build-metrics", help="Build metrics directory"
    )
    parser.add_argument(
        "--results-dir", default="all-macro-results", help="Macro results directory"
    )
    parser.add_argument(
        "--config",
        default="firmware/RAMNV1/Core/Inc/ramn_config.h",
        help="ramn_config.h path",
    )
    parser.add_argument("--total-lines", default="0", help="Total .c lines")
    parser.add_argument("--compiled-lines", default="0", help="Compiled .c lines")
    parser.add_argument("--coverage-pct", default="0", help="Coverage percentage")
    parser.add_argument("--output", default="report.md", help="Output file")
    args = parser.parse_args()

    sizes = parse_hex_sizes(args.metrics_dir)
    macro_results = parse_macro_results(args.results_dir)
    macros = scan_macros(args.config)

    # Count covered macros
    targets = ["TARGET_ECUA", "TARGET_ECUB", "TARGET_ECUC", "TARGET_ECUD"]
    variants = []
    for rec in macro_results:
        ecu = rec.get("ecu", "")
        variant = rec.get("variant", "")
        enable = set(rec.get("enable", "").split()) - {""}
        disable = set(rec.get("disable", "").split()) - {""}
        variants.append((ecu, variant, enable, disable))

    covered_count = 0
    total_macros = len(macros)
    for macro, states in macros:
        default_on = [t for t in targets if states.get(t, False)]
        default_off = [t for t in targets if not states.get(t, False)]
        variant_on = any(macro in e for _, _, e, _ in variants)
        variant_off = any(macro in d for _, _, _, d in variants)
        has_on = bool(default_on) or variant_on
        has_off = bool(default_off) or variant_off
        if has_on and has_off:
            covered_count += 1

    if total_macros > 0:
        coverage_macro_pct = round(covered_count * 100.0 / total_macros, 1)
    else:
        coverage_macro_pct = 0

    with open(args.output, "w") as f:
        f.write("<!-- build-coverage-report -->\n")
        f.write("# 🔨 Build & Macro Coverage Report\n\n")
        f.write("## Default Builds \u2014 Hex File Sizes\n\n")
        f.write(hex_size_table(sizes) + "\n\n")
        f.write("## Macro Coverage Build Results\n\n")
        f.write(macro_results_table(macro_results) + "\n\n")
        f.write("## Source Code Compile Coverage\n\n")
        f.write(f"- **Total .c source lines:** {args.total_lines}\n")
        f.write(
            f"- **Lines compiled in \u22651 configuration:** "
            f"{args.compiled_lines} (~{args.coverage_pct}%)\n"
        )
        f.write(
            f"- **ENABLE_ macro coverage:** {covered_count}/{total_macros} "
            f"({coverage_macro_pct}%) macros tested in both ON and OFF states\n\n"
        )
        f.write("<details>\n")
        f.write("<summary>Full ENABLE_ Macro Coverage Table</summary>\n\n")
        f.write(build_macro_coverage_table(macros, macro_results) + "\n\n")
        f.write("</details>\n")


if __name__ == "__main__":
    main()
