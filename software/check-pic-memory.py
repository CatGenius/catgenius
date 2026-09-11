#!/usr/bin/env python3
"""Check XC8's linked memory usage and conservative hardware-stack budget."""
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


def check(device, xml, map_text, summary):
    limits = {"16F1939": (16384, 1024, 16, 2), "16F877A": (8192, 368, 8, 1)}
    program, data, stack, config = limits[device]
    root = ET.fromstring(xml)
    reports = []
    for name, capacity, units in (("program", program, "words"), ("data", data, "bytes")):
        memory = root.find(f"./executable/memory[@name='{name}']")
        if memory is None or memory.findtext("units") != units:
            raise ValueError(f"Missing or unsupported {name} memory report")
        used = int(memory.findtext("used", "-1"))
        if int(memory.findtext("length", "0")) != capacity or not 0 <= used <= capacity:
            raise ValueError(f"Invalid or over-budget {name} allocation")
        reports.append(f"{name}: {used}/{capacity} {units}; {capacity - used} free")
    config_used = re.search(r"Configuration bits\s+used\s+\w+h\s+\(\s*(\d+)\)", summary)
    if not config_used or int(config_used[1]) != config:
        raise ValueError("Missing device configuration words")
    guidance = map_text.split("STACK USAGE GUIDANCE", 1)[1].split("STACK USAGE ESTIMATES", 1)[0]
    if "The program uses a compiled stack model." not in guidance:
        raise ValueError("The data stack must be link-time allocated")
    main = re.search(r"uses an estimated (\d+) of (\d+) hardware stack", guidance)
    if not main or int(main[2]) != stack:
        raise ValueError("Missing or unsupported main stack estimate")
    if "Recursive" in guidance or "external function" in guidance.lower():
        raise ValueError("Stack guidance needs manual review")
    interrupts = re.findall(r"call-graph uses an estimated (\d+) hardware stack", guidance)
    if len(interrupts) > 1:
        raise ValueError("Unexpected nested-interrupt model")
    irq = int(interrupts[0]) if interrupts else 0
    # Allow optimizer-generated calls in both main and interrupted contexts.
    optimizer = int("optimization level might use a level of hardware stack" in guidance) * (1 + bool(irq))
    total = int(main[1]) + irq + optimizer
    reports.append(f"hardware stack: {main[1]} main + {irq} interrupt + {optimizer} optimizer = {total}/{stack}")
    if total > stack:
        raise ValueError("; ".join(reports) + "; hardware-stack budget exceeded")
    if total == stack:
        reports.append("WARNING: no conservative hardware-stack headroom remains")
    return reports


if __name__ == "__main__":
    try:
        device, application = sys.argv[1:]
        reports = check(device, Path("memory.xml").read_text(),
                        Path("firmware.map").read_text(), Path(application + ".mum").read_text())
    except (ValueError, KeyError, IndexError, OSError, ET.ParseError) as error:
        sys.exit(f"PIC resource gate FAILED: {error}")
    print("PIC resource gate passed (not appliance validation):")
    print("\n".join(reports))
