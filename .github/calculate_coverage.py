#!/usr/bin/env python3
"""Calculate .c line coverage across all build configurations.

Parses ramn_config.h to derive which macros are active per ECU target,
then evaluates #if/#ifdef/#ifndef/#else/#elif/#endif blocks in each .c
file to determine which lines are compiled in at least one configuration.

Usage: python3 calculate_coverage.py [config_path] [src_dir]
Output: key=value pairs (total_lines, compiled_lines, coverage_pct)
"""

import glob
import os
import re


def parse_config(path):
    """Parse ramn_config.h to extract active #define macros per ECU target."""
    with open(path) as f:
        lines = f.readlines()
    targets = {
        t: set() for t in ["TARGET_ECUA", "TARGET_ECUB", "TARGET_ECUC", "TARGET_ECUD"]
    }
    common = set()
    current_target = None
    depth = 0
    target_depth = 0
    guard_depth = 0
    for line in lines:
        s = line.strip()
        if re.match(r"#\s*(?:ifdef|ifndef|if)\b", s):
            depth += 1
            # Skip the include guard (#ifndef INC_RAMN_CONFIG_H_)
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
        m = re.match(r"#\s*define\s+(\w+)", s)
        if m:
            macro = m.group(1)
            if macro.startswith("INC_"):
                continue
            if current_target:
                targets[current_target].add(macro)
            elif depth == guard_depth:
                common.add(macro)
    # Build base configs with derived macros
    configs = {}
    for t in targets:
        d = targets[t] | common | {t}
        if d & {"ENABLE_UDS", "ENABLE_KWP", "ENABLE_XCP"}:
            d |= {"ENABLE_DIAG", "ENABLE_EEPROM_EMULATION"}
        if d & {"ENABLE_UDS", "ENABLE_KWP"}:
            d.add("ENABLE_ISOTP")
        if t != "TARGET_ECUA":
            d.add("USE_HARDWARE_CAN_FILTERS")
        configs[t] = d
    return configs


def derive(macros):
    """Re-derive dependent macros after adding/removing flags."""
    d = set(macros)
    d -= {"ENABLE_DIAG", "ENABLE_EEPROM_EMULATION", "ENABLE_ISOTP"}
    if d & {"ENABLE_UDS", "ENABLE_KWP", "ENABLE_XCP"}:
        d |= {"ENABLE_DIAG", "ENABLE_EEPROM_EMULATION"}
    if d & {"ENABLE_UDS", "ENABLE_KWP"}:
        d.add("ENABLE_ISOTP")
    return d


def eval_cond(expr, defs):
    """Evaluate a preprocessor #if expression.

    Handles defined(X), !defined(X), ||, &&, and constant expressions.
    Returns True for expressions that can't be fully evaluated (e.g.
    comparisons involving macro values like LED_TEST_DURATION_MS > 0),
    which is conservative — assumes the code is compiled.
    """
    e = re.sub(
        r"defined\s*\(\s*(\w+)\s*\)", lambda m: "1" if m.group(1) in defs else "0", expr
    )
    e = re.sub(r"defined\s+(\w+)", lambda m: "1" if m.group(1) in defs else "0", e)
    e = e.replace("||", " or ").replace("&&", " and ")
    e = re.sub(r"!\s*\(", "not (", e)
    e = re.sub(r"!(?!=)\s*([01])", lambda m: "1" if m.group(1) == "0" else "0", e)
    try:
        return bool(eval(e))
    except (SyntaxError, NameError, TypeError, ValueError):
        return True


def coverage(files, all_configs):
    """Return (total_lines, compiled_lines_set) across all configs."""
    total = 0
    compiled = set()
    for fp in files:
        with open(fp) as f:
            flines = f.readlines()
        total += len(flines)
        for defs in all_configs:
            # Stack entries: (parent_compiling, this_branch_active, any_branch_taken)
            st = [(True, True, True)]
            for i, line in enumerate(flines, 1):
                s = line.strip()
                if re.match(r"#\s*ifdef\s", s):
                    p = st[-1][0] and st[-1][1]
                    macro = s.split()[1] if len(s.split()) > 1 else ""
                    a = macro in defs
                    st.append((p, p and a, a))
                elif re.match(r"#\s*ifndef\s", s):
                    p = st[-1][0] and st[-1][1]
                    macro = s.split()[1] if len(s.split()) > 1 else ""
                    a = macro not in defs
                    st.append((p, p and a, a))
                elif re.match(r"#\s*if\s", s) and not re.match(r"#\s*if(n?)def\s", s):
                    p = st[-1][0] and st[-1][1]
                    expr = re.sub(r"^#\s*if\s+", "", s)
                    a = eval_cond(expr, defs) if p else False
                    st.append((p, p and a, a))
                elif re.match(r"#\s*elif\s", s):
                    if len(st) > 1:
                        p, _, taken = st.pop()
                        if not taken and p:
                            expr = re.sub(r"^#\s*elif\s+", "", s)
                            a = eval_cond(expr, defs)
                        else:
                            a = False
                        st.append((p, p and a, taken or a))
                elif re.match(r"#\s*else\b", s):
                    if len(st) > 1:
                        p, _, taken = st.pop()
                        st.append((p, p and not taken, True))
                elif re.match(r"#\s*endif\b", s):
                    if len(st) > 1:
                        st.pop()
                else:
                    if st[-1][0] and st[-1][1]:
                        compiled.add((fp, i))
    return total, compiled


def main():
    import sys

    config_path = (
        sys.argv[1] if len(sys.argv) > 1 else "firmware/RAMNV1/Core/Inc/ramn_config.h"
    )
    src_dir = sys.argv[2] if len(sys.argv) > 2 else "firmware/RAMNV1/Core/Src"
    results_dir = sys.argv[3] if len(sys.argv) > 3 else "all-macro-results"

    base = parse_config(config_path)
    # 4 default ECU configs
    all_configs = list(base.values())

    # Read macro coverage variants from macro-results artifacts
    if os.path.isdir(results_dir):
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
            ecu = rec.get("ecu", "")
            enable = set(rec.get("enable", "").split()) - {""}
            disable = set(rec.get("disable", "").split()) - {""}
            if ecu in base:
                all_configs.append(derive((base[ecu] | enable) - disable))

    files = sorted(glob.glob(os.path.join(src_dir, "*.c")))
    total, compiled = coverage(files, all_configs)
    pct = round(len(compiled) * 100.0 / total) if total > 0 else 0
    print(f"total_lines={total}")
    print(f"compiled_lines={len(compiled)}")
    print(f"coverage_pct={pct}")


if __name__ == "__main__":
    main()
