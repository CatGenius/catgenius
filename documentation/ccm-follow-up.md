# Remaining work from the ccm branch

Reviewed on 11 September 2026: `origin/ccm` at
`4494e330177c34e2c757877b72b1a41e0e232840`, dated 7 April 2018, against B07 at
`113de13`. There are 41 commits on the ccm side after the common ancestor
`48ca457`; 11 have patch-equivalent commits in B07's ancestry. Other changes
were copied selectively, so a different commit ID does not imply missing work.

This is an inventory of useful ideas, their implementation limits and possible
follow-up work, not a commitment to enable every feature. Only terminal editing
has been ported as a result of this review. The other features below remain
unimplemented in B07 unless explicitly identified as already recovered.

Sources below are Git objects, not paths to files in the B07 checkout. Inspect
them without checking out or merging the branch, for example:

```sh
git show 4494e33:software/common/eventlog.c
git show 5dc2fa0 -- software/catgenius/litterlanguage.c
```

## Ported: terminal editing

The editing behavior from Christopher Mapes' integration commit `b0b7b14` is
now adapted to the existing command parser in `d5d26a9`. Backspace (0x08) and
Delete (0x7f) both remove the last buffered character. With local echo enabled,
the editor sends Backspace, space, Backspace to visibly erase it. An empty line
produces no erase sequence; with echo disabled, edits still work without
terminal output.

The argument-count guard, empty-input handling and dispatch after character
handling returns are retained. This does not import the old parser, change
line-termination rules or enable any additional commands. The
[host tests](../software/tests/README.md) capture exact output and exercise
both erase keys, buffer boundaries, split input and echo-off operation.

## Open: per-application feature configuration

Sources: `b0b7b14`, `9c7e474`, `13fca93`, `4818451`, `4494e33`;
`software/common/app_prefs.h` and the application-specific `*_prefs.h` files.

The branch separates optional serial output, debug logging, command families,
RTC, Bluetooth, event reporting and integrated diagnostics. It also expresses
dependencies, such as tag commands requiring CR14 and I2C support, and disables
some features on the smaller 877A.

This is the highest-priority architectural candidate for recovering space and
keeping wash-controller and service-tool configurations distinct. Adapt the
idea to the current XC8 build, with checked dependencies and explicit profiles;
do not copy the default enabled-feature set. Keep safety behavior independent
of optional diagnostics. Making the RTC optional must not disable the timer
services used by the wash controller.

The integration is incomplete. In the reviewed tip, the GenieDiag selection in
`app_prefs.h` includes `../catgenius/geniediag_prefs.h`, although the file lives
under `geniediag/`. Older compiler declarations and project files also require
adaptation. The word counts in `catgenius_prefs.h` and the savings recorded in
`9c7e474` are historical measurements, not current XC8 budgets or additive costs.

Before adoption: build and test each supported application/device/profile,
check disabled-feature dependencies, and measure flash, RAM and hardware-stack
usage. This inventory does not establish that a smaller 877A build will fit.

## Open: additional serial commands

Source: `b0b7b14`, `software/common/cmdline.c` and the CatGenius command table.

The branch adds `mode`, `start`, `setup`, `lock` and `cart` commands, including
short/long button-action selection. It also makes the existing peripheral,
GPIO and tag commands available from the wash-controller application instead
of only from separate service tools. The remote UI controls could support
repeatable testing and external automation.

The extra commands call gesture handlers directly and expose UI state across
modules. Numeric and textual argument validation is loose. Before adoption,
define which commands are read-only, when control commands are permitted, and
how they interact with locking, faults, diagnostics and an active program.
Use bounded, validated control interfaces rather than direct writes to UI
globals or uncoordinated actuator access. Preserve the current parser guards
and command-dispatch depth; add tests for invalid parameters and every allowed
state transition. Do not enable raw GPIO control in an ordinary wash profile
merely because the feature switch exists.

## Open: live status reporting

Sources: `b0b7b14`, `7a834c5`, `ff3f8d6`;
`software/common/eventlog.c`, `eventlog.h` and their sensor/actuator call sites.

The branch supplies serial status messages for actuators, sensors, buttons,
LED/pacer settings and the current instruction address. An `evt` command
enables reporting. This is useful for remote observation and fault diagnosis.

Despite its name, `eventlog` is not a chronological or persistent incident log.
It caches 23 unsigned 16-bit values, consuming 46 RAM bytes for that array alone.
It reports changes while enabled and sends nonzero cached values when enabled;
there are no timestamps, complete replay or durable records. Some direct GPIO
updates are not represented. It is not a replacement for B07's deferred fault
assertion/clear handling.

The water reporting changes suppress variations of four ADC counts or less,
with a sample-count fallback for smaller accumulated changes. The fallback
still does not send an unchanged value, so it is not a guaranteed heartbeat.
Its nominal minute is tied to the old sampling interval. Preserve the idea of
throttled telemetry, not its sampling implementation or constants; reporting
must not suppress measurements used for control or quality qualification.

Before adoption: define a complete initial snapshot, stable event identifiers,
measurement validity/context and reporting cadence. Move bounded serial output
out of actuator/fault call chains to a worker, and define behavior for slow or
disconnected receivers. Check timing and call depth as well as memory. Treat
timestamped incident history as a separate, still-open feature.

## Open: integrated service diagnostics

Sources: `42be6ce`, followed by the conditional-compilation fixes in `c46e32d`;
`software/catgenius/userinterface.c` and `litterlanguage.c`.

Long Setup enters/exits a diagnostic panel. Short Setup selects bowl, arm,
dosage, drain pump, dryer or water; short Start operates the selected output.
The panel also displays sensor states. This could eliminate reflashing merely
to exercise peripherals.

The implementation pauses the interpreter and later restores its previous
pause state. It suppresses normal water/overheat event actions in diagnostics,
uses direct actuator calls, and lacks a complete mode-exit and fault policy.
Automatic scheduling and other button actions also need explicit ownership.
These behaviors must not be imported as a shortcut around B07's protections.

Before adoption: specify entry conditions, suspend automatic starts, enforce
the key lock, bound actuator on-times, and explicitly stop service outputs on
exit or fault. Decide whether a previously running program may resume and what
must be revalidated first. Adapt water diagnostics to current acquisition and
quality state. Test idle, running, paused and faulted entry/exit combinations.

## Open: arm run-time limits and estimated positioning

Sources: `9f6ab6e`, `5dc2fa0`, `b5c9861`; `CMM_ARM_EXPERIMENT` in preferences,
`software/common/catgenie120.c`, `cmdline_box.c`, and the interpreter/recipes.

There are two separable ideas:

- A timer that stops an arm movement after a bounded duration. This is useful
  to consider independently of position-oriented recipes.
- An estimated 0..100 percent arm position, calculated from elapsed motor time
  using a nominal 13.5-second stroke. Moves to an endpoint add extra time to
  try to restore the estimate. No physical position feedback is added.

The estimate is not proof of position: interruptions, starting in an unknown
position, stalled motion and calibration errors can invalidate it. Overflow,
pause/resume behavior and 50/60 Hz timing require review. The timeout is serviced
by the main loop, not an independent hardware cutoff; the branch's early return
while paused also limits its coverage of manually commanded movement.

The instruction contract changes: `INS_ARM` operands 0..100 become target
positions, with 253/254/255 representing up/down/stop. In B07, operand 0 means
stop. Importing only the interpreter would therefore change existing recipes'
meaning. The updated recipes still largely use direction/stop instructions,
not a demonstrated complete conversion to position-based operation.

Before adoption: first evaluate a separate run-time guard against every current
recipe and service path. Keep position control experimental until calibrated
and tested. A future instruction-format change requires coordinated recipe,
interpreter and tool updates, explicit compatibility handling, and validation.

## Open: host-side LitterLanguage tools

Source: `b0b7b14`, `software/llc/Program.cs`.

The C# prototype converts C recipe arrays to a readable language, emits a
three-byte instruction representation, and converts binary instructions back
to C. It contains useful ideas for named subroutines, readable delays and
splitting long waits. Host-side generation/validation need not consume PIC RAM
or program space beyond the resulting recipes.

It is not ready to trust as a compiler. Development paths are hard-coded;
preprocessor handling is incomplete; `//` removal retains the comment instead
of the preceding code; forward-label resolution is unfinished. Error paths can
leave partial output. Byte-offset call targets are not automatically compatible
with the current PIC ROM-pointer representation.

Before adoption: define the language/encoding, implement deterministic parsing
and relocation, reject invalid programs without publishing partial artifacts,
and test generated instructions against known recipes. Check branch targets,
wait/dose ranges, wet/dry paths and the interpreter's single return-address
limit. Do not execute the checked-in development binaries as a substitute for
building and testing source. External EEPROM/tag program loading remains a
separate missing firmware feature; this tool does not implement it.

## Optional: Bluetooth and communication tests

Sources: `bdfb6a0`, `57edc81`, `b0b7b14`, `9c7e474`;
`software/common/bluetooth.c`, `serial.c`, `serial.h` and `cmdline.c`.

Useful ideas include probing several baud rates, separating module setup from
normal flow control, using constant baud-rate strings instead of floating-point
formatting, and measuring serial receive/transmit behavior. Bluetooth and the
communication tests are disabled in the reviewed CatGenius preferences.

Known obstacles include a 16-bit baud-rate table that cannot represent 115200,
an unchecked write into a ten-byte scratch buffer in `serial_wait_s()`, and
incorrect response matching caused by the placement of an `else`. Partial
matches are also accepted as successful replies. Module command construction
and initialization/interrupt ordering need checking. The declared serial
line-termination configuration is not consumed by the final serial driver.

The receive/transmit tests block normal processing and use floating-point
reporting. Recover them as host-driven or dedicated service tests with bounded
execution, not commands that can monopolize an active wash controller. There
is no need to enable Bluetooth to recover terminal editing or serial telemetry.

## Already recovered or superseded

| Work in ccm | Existing main-line/B07 history |
| --- | --- |
| Serial buffering and flow-control corrections (`5e3d866`, `262eca7`) | `11a9e71`, `3c5e61f`; the underlying receive buffering is also already present |
| Pacer/debouncer memory reductions (`a98b86b`, `6f4ccbc`) | `8ef0769`, `429d993`, followed by B07's input normalization fixes |
| Separate key and error beeps (`017304c`) | `6665c40` |
| Water reflection callbacks (`01873de`) | `6a27a83`; B07 subsequently adds separate qualified level and optical-quality handling |
| Overheat argument normalization (part of `7a834c5`) | `62a0d9c`, with B07's independent active-program protection |
| 12/24-hour timeout workaround (`84b3a88`) | `ed4d29d` |
| Earlier pause and error handling | Already in the main lineage; B07 fixes restoration, unbounded waits and non-reentrant fault delivery |
| MPLAB project conversions | Superseded by the current XC8-only build and resource checks |

Individual historical fixes still need review. For example, `7a834c5` also
moves heat detection to a PORTB change mask instead of the actual input level;
that part must not be copied with the useful argument normalization. Likewise,
the old parser lacks B07's argument bounds and empty-input guard, and its nested
dispatch costs an extra call level. The old water acquisition, button behavior
and synchronous error callbacks must not replace the current implementations.

The extra tag/CR14 commands mostly expose functionality already available in
the service tools. They do not complete cartridge accounting, dosage policy or
external recipe loading. The branch does not close those outstanding items.

## Integration order and validation limits

Terminal editing is complete. Suggested subsequent work is feature profiles,
then a separately reviewed arm run-time guard. Diagnostics, live status and
recipe tooling are independent follow-ups; Bluetooth is optional.

Keep implementation changes and bug fixes in reviewable commits rather than
cherry-picking the broad `b0b7b14` integration wholesale. Preserve existing
firmware style/CRLF and use LF for new files. Do not restore PICC support.

Re-run both host compilers and the application/device resource matrix after
firmware changes. The [XC8 report](pic-build.md) records small hardware-stack
margins and unresolved 877A allocation failures. An extra reporting call chain
can exhaust stack headroom even when flash and RAM appear sufficient. This
review did not build or operate ccm, and no feature described here constitutes
hardware validation or permission to flash an appliance.
