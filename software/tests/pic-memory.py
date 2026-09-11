"""Test the resource gate independently of the proprietary target compiler."""
import importlib.util
from pathlib import Path

spec = importlib.util.spec_from_file_location("pic_memory", Path(__file__).parents[1] / "check-pic-memory.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
xml = """<project><executable>
<memory name="program"><units>words</units><length>16384</length><used>11543</used></memory>
<memory name="data"><units>bytes</units><length>1024</length><used>449</used></memory>
</executable></project>"""
guidance = """STACK USAGE GUIDANCE
The program uses a compiled stack model.
uses an estimated 12 of 16 hardware stack locations
isr() call-graph uses an estimated 2 hardware stack locations
Current optimization level might use a level of hardware stack.
STACK USAGE ESTIMATES"""
summary = "Configuration bits   used 2h ( 2) of 2h words"
assert "WARNING" in module.check("16F1939", xml, guidance, summary)[-1]
plain = guidance.replace("12 of", "13 of").replace(
    "Current optimization level might use a level of hardware stack.", "")
assert "15/16" in module.check("16F1939", xml, plain, summary)[-1]
legacy_xml = xml.replace("16384", "8192").replace("11543", "4068").replace("1024", "368").replace("449", "278")
legacy_map = plain.replace("13 of 16", "8 of 8").replace(
    "isr() call-graph uses an estimated 2 hardware stack locations", "")
assert "WARNING" in module.check("16F877A", legacy_xml, legacy_map, summary.replace("( 2)", "( 1)"))[-1]
for altered_xml, altered_map, altered_summary in (
    (xml.replace("11543", "17000"), guidance, summary),
    (xml.replace("449", "1025"), guidance, summary),
    (xml.replace("16384", "32768"), guidance, summary),
    (xml, guidance.replace("12 of", "13 of"), summary),
    (xml, "", summary),
    (xml, guidance, summary.replace("( 2)", "( 0)")),
    (xml, guidance.replace("compiled", "software"), summary),
):
    try:
        module.check("16F1939", altered_xml, altered_map, altered_summary)
    except (ValueError, IndexError):
        pass
    else:
        raise AssertionError("Resource gate accepted an invalid report")
print("PIC resource-gate checks passed.")
