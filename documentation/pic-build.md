# XC8 builds and resource checks

XC8 is the supported compiler. PICC support was retired independently in
`e047d3a`. The host tests do not substitute for these target builds.

## Reproduce a build

Tested with XC8 4.00 and these Microchip device packs:

- PIC16F1939: `PIC12-16F1xxx_DFP` 1.9.258.
- PIC16F877A: `PIC16Fxxx_DFP` 1.9.176.

Obtain the [compiler](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers/xc8)
and [device packs](https://packs.download.microchip.com/) from Microchip.
`PIC_DFP` must point to the selected pack's `xc8` directory, not its parent.
The 877A pack archive used here has SHA-256
`c5c3ce16f908635a6858fc128494b82ae9aa61b7f5ab932a12843bbdfa8f8f62`.

From the repository root, for example:

```sh
pic_build_root=$(mktemp -d /tmp/catgenius-pic.XXXXXX)
XC8_CC=/opt/cross/xc8-v4.00/bin/xc8-cc \
PIC_DFP=/opt/cross/microchip-packs/Microchip/PIC12-16F1xxx_DFP/1.9.258/xc8 \
sh software/build-pic.sh catgenius 16F1939 "$pic_build_root/catgenius-16F1939"
```

Applications are `catgenius`, `geniediag` and `iotester`; devices are `16F1939`
and `16F877A`. Select the matching pack and use a **new output directory** each
time. An existing directory is refused, preventing stale successful artifacts
from being mistaken for a later failed build. Python 3 checks the reports.

The script uses C90, `-O1`, the compiled/non-reentrant data stack, and the
existing production configuration pragmas. It does not define `__DEBUG`,
change EEPROM contents, enlarge the device's memory or remove features to fit.
Existing 1939 diagnostic logging remains enabled. `PIC_OPTIMIZATION` can select
another optimization level for comparisons; that is a different build profile.

Outputs include the ELF/HEX, `firmware.map`, `memory.xml` and the compiler's
`.mum` memory summary. Preserve the console output as well. A successful
compile/link is followed by checks of actual device capacities, configuration
word presence, the compiled data-stack model and hardware-stack estimates.
Compiler/linker failures or an over-budget estimate return a nonzero status.
The stack check adds the ISR depth and, when flagged by XC8, an extra optimizer
level for each of the main and interrupt contexts. Exact-capacity estimates
warn because they leave no headroom. This is a conservative development gate,
not proof of actual runtime stack depth or electrical safety.

The MPLAB descriptors now select XC8 4.00 and `-O1` and no longer contain
machine-specific PICC include paths. Some XML property group names still say
`HI-TECH`; they are legacy IDE identifiers, not a second supported compiler.
MPLAB IDE regeneration/validation of its generated makefiles was not performed.
The command-line script is the tested, reproducible build route.

## Results on 11 September 2026

Firmware through `405fac6`, built with the default XC8 4.00 / `-O1` profile:

| Application | Device | Program words | RAM bytes | Hardware-stack estimate | Result |
| --- | --- | ---: | ---: | ---: | --- |
| CatGenius | 16F1939 | 13,140 / 16,384 (80.2%) | 458 / 1,024 (44.7%) | 13 main + 2 ISR = 15 / 16 | Fits; one estimated stack level spare |
| GenieDiag | 16F1939 | 10,883 / 16,384 (66.4%) | 382 / 1,024 (37.3%) | 13 main + 2 ISR = 15 / 16 | Fits; one estimated stack level spare |
| IOTester | 16F1939 | 3,757 / 16,384 (22.9%) | 297 / 1,024 (29.0%) | 8 / 16 | Fits |
| CatGenius | 16F877A | No completed link | Allocation fails | Unavailable | Blocked by RAM allocation |
| GenieDiag | 16F877A | Allocation fails | No successful whole-image report | Not qualified | Blocked by program-space allocation |
| IOTester | 16F877A | 4,068 / 8,192 (49.7%) | 278 / 368 (75.5%) | 8 / 8 | Fits numerically; no estimated stack headroom |

The 877A failures also occurred before this behavioral-fix batch under XC8:
they must not be described as newly introduced by the button or timer changes,
nor as resolved by passing the host suite. The RAM error names the first object
that cannot be placed; it is **not** a reliable measurement of the total excess.

Rebuilding the pre-behavioral-fix snapshot `e047d3a` with the same `-O1`
profile gives CatGenius/1939 13,007 words and 466 RAM bytes: this batch adds
133 words and saves 8 RAM bytes. GenieDiag goes from 10,787 words / 383 bytes
to 10,883 / 382, while its main call-depth estimate drops from 14 to 13.
IOTester/1939 goes from 3,730 / 298 to 3,757 / 297.

For comparison, `-Os` builds CatGenius/1939 at 11,543 words and 449 RAM bytes,
but its 13-level main estimate plus 2 ISR levels and optimizer allowances does
not pass the conservative stack gate. GenieDiag has the same stack concern.
The default deliberately spends some flash to avoid this optimizer allowance.
The pre-batch `-Os` baseline after compiler compatibility fixes used 11,433
words / 458 bytes for CatGenius: the behavioral changes add 110 words and save
9 bytes under that same profile. Do not compare different optimization levels
as though their size difference came solely from source changes.

## Remaining release gates

Neither an image nor its memory report is permission to flash an appliance.
No image was flashed or operated during this work. The 1939 hardware-stack
margin is small, and the 877A wash and diagnostic images still need a dedicated
size/call-depth pass. Inspect generated call paths, interrupt latency, shared
GPIO instructions, the instruction-pointer conversion and complete wash
recipes before controlled hardware testing. Sensor thresholds and electrical
behavior remain unvalidated. Do not bypass the resource gate to label a
failing configuration supported.
