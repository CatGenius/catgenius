# B07 development

Started on 11 September 2026 from `master-b06` at `e5c34b0`. The historical
`b06` tag stays at `ff3785a`, before the two recent UI fixes. This is development
work, not a release or a claim that an appliance has been validated.

For a fresh session, start with the [roadmap](#roadmap) and
[session handoff](#session-handoff) below.

## Implemented

The first checkpoint fixed button/overheat normalization, lost pause context,
fill restoration after high water arrived while paused, and unbounded water
waits. These fixes remain separate commits with their regression tests.

The next batch fixes the held-button key-lock bypass, command argument bounds
and empty input, idle pause contexts and overheated starts, pending Timer1
overflows, and the hour-setting helper. Boolean fault assertions/final clears
now use two byte masks instead of a ten-byte queue, so bursts cannot discard a
fault. Repeated states coalesce; assertions still take effect before their final
clear. Different types are handled in event-number order and callbacks remain
deferred to the next pass. This is not a chronological fault history or a full
multi-fault display/acknowledgement policy.

PICC support was retired in stand-alone commit `e047d3a`. Declaration fixes,
behavioral changes and build tooling have separate commits. A command-line XC8
build and resource gate now report flash, RAM and hardware-stack estimates;
see [build instructions and measured limits](pic-build.md).

The terminal editor now accepts Backspace (0x08) as well as Delete (0x7f),
visibly erasing the last character when local echo is enabled. Empty-line and
full-buffer boundaries, split input and echo-off behavior have exact-output
regression tests. The existing parser guards and reduced dispatch depth remain.

Water handling now includes:

- Qualified water-level readings, acquisition watchdogs and retained diagnostic
  measurements. An acquisition timeout inhibits filling and stops active or
  paused programs with panel error 4, retaining the persisted box state.
- Dryer startup, runtime and resume guards requiring qualified low water.
  Blocked dryer instructions are retained for explicit retry, using panel
  error 2. A low-water indication is not proof that every part has drained.
- On the 16F1939, four-reading AN1 means and separate RB3 comparator-based
  level detection. Timer4 ends the nominal 0.5 ms valve-enable probe in an ISR,
  independently of main-loop processing.
- Separate optical-quality confirmation, threshold/history handling and
  consecutive-good recovery. An optical fault is not conclusively a dirt
  diagnosis. Quality testing is not applied to ordinary submerged readings
  during a wash.
- A bounded preflight for full 16F1939 washes. Quality and a fresh low-water
  indication must pass before actuator instructions run. Failures stop the
  request; recovery does not restart it. Scoop-only and recovery-cleanup paths
  remain available.
  Failed/cancelled preflight leaves a tidy-box record unchanged; the transition
  to messy is recorded only once the preflight succeeds.
- Read-only diagnostics distinguishing quality states, ADC/probe timeouts,
  the last complete mean and fill context, the last enabled comparator sample,
  and live output states.

The 16F877A retains its digital RA1 sensing and existing program-start path,
without Timer4 probes or analogue quality checks. Its diagnostic data remains
explicitly labeled digital. The [water-sensing specification](water-sensing.md)
documents timing, fault/recovery behavior, interfaces and validation limits.

## Unchanged or still open

- Wash and timed-drain recipes, dosage calibration, 50/60 Hz compensation,
  cartridge volume/accounting, cartridge-type behavior, cat-triggered dose
  selection and scent policy.
- Per-board calibration and acquisition accuracy. The default quality
  threshold of 410 is configurable, but not established as correct for every
  board. No EEPROM layout or calibration bytes were changed.
- Measured probe width and electrical effects under load, relay response,
  optical settling and thresholds for clean/dry, wet, submerged, dirty and
  misaligned conditions. The main-loop watchdog is not a hardware safety
  cutoff; Timer4 pulse termination still depends on interrupt service.
- Timestamped fault history, stale-value detection beyond acquisition timeouts,
  and a universal low-level actuator interlock. GenieDiag and IOTester remain
  service tools, not automatically safe wash controllers.
- Idle flood handling and a complete fault/acknowledgement policy, optional
  features from the closure inventory, and remaining timer representation/
  extreme-uptime limitations. No automatic idle drainage was introduced.
- The 877A CatGenius RAM and GenieDiag program-space allocation failures under
  XC8, and small hardware-stack margins. No features were removed to force a fit.

The [ccm follow-up inventory](ccm-follow-up.md) records the remaining feature
configuration, serial commands, diagnostics, status reporting, arm-control,
recipe-tooling and Bluetooth work, with source commits, blockers and validation
requirements.
Only terminal editing was ported from that review; the rest remains deferred.

## Roadmap

Proposed order as of 11 September 2026. These are open tasks, not completed
features or a commitment to implement every historical wish. Keep this as the
central priority list; record completion with commit references and verification
results. Detailed coverage and limitations belong in the
[test README](../software/tests/README.md).

1. [ ] Automatic-mode tests, the next implementation tranche. Exercise cat
   arrival/departure, the four-minute waiting period and returning cats; timed
   schedules; and full-wash/scoop-only ratios across modes 0..9. Include manual
   starts, locking, pauses and faults during automatic operation. Characterize
   how aborted or failed attempts affect scheduling/counting, and confirm the
   intended policy before changing that behavior.
2. [ ] Power-loss recovery tests. Interrupt preflight, filling, dosing, draining,
   drying and paused execution; preserve that run's EEPROM contents and start
   a fresh process. Check tidy/messy/wet cleanup selection, actuator state and
   persisted markers. This validates existing recovery, not a new feature for
   resuming at the exact interrupted instruction.
3. [ ] Complete wash-recipe validation. Add the actual wash/cleanup recipes
   alongside the short synthetic scenarios. Validate wet/dry paths, calls,
   returns and skips, actuator sequencing and total dosage-pump on-time. Use
   target-aware call-address handling; a host address mapping alone does not
   validate the PIC instruction-pointer conversion or ABI.
4. [ ] Automated regression runs on pushes and pull requests. Run the host
   suite with GCC and Clang and both sanitizers. Keep host results distinct
   from PIC resource checks; automated XC8 builds require the compiler and
   matching device packs to be provisioned explicitly.
5. [ ] Resource headroom before further firmware features. Review generated
   call paths and interrupt depth, the 1939 stack margin and the 877A allocation
   failures. Evaluate the feature-profile idea from the
   [ccm inventory](ccm-follow-up.md#open-per-application-feature-configuration)
   without weakening control protections. Rebuild the application/device matrix
   and record measured results; do not bypass failing gates or silently drop
   target support. This gate need not delay test-only work above.

Further independent candidates include real serial/cat-sensor driver fault
tests and the optional features in the ccm inventory. Before any release, the
hardware and build gates below still apply. Passing host scenarios alone is
not sufficient for release.

## Verification and next gate

Run [the host regression checks](../software/tests/README.md) with GCC and Clang.
The quality-decision tests cover every combination of 0..10 analogue and
comparator failures in the repeat window, history boundaries and recovery.
The firmware tests exercise actual changed C paths with supplied inputs for
both PIC configurations, including the Timer4 handler and wash preflight.
The integrated fixture additionally links the real UI, interpreter, board,
water, quality, RTC and command parser, using the application's worker order.
It runs 12 scenarios on each configuration plus 11 analogue-only scenarios,
covering button/fault timing, dosing across pauses, interlocks, preflight and
acquisition failures, and deferred UI handling. Failures include a timestamped
state-change trace. Each scenario starts in a fresh process without modifying
private firmware state or adding production reset hooks.
As a negative control, removing only the hot-start guard in a temporary source
copy makes the simultaneous Start/overheat scenario fail at the shared event
pass. The unmodified source passes all 35 scenario executions with both compilers.

On 11 September 2026, every new commit passed the checks with both compilers,
using address/undefined-behavior sanitizers. The quality-decision module also
passes a strict C90 syntax check. Both edited MPLAB configuration XML files
parse successfully. Whitespace checks recognize CRLF: existing firmware and
project files retain their line endings; new source, test and documentation
files use LF. Test-directory attributes enforce LF for the framework.

These are not PIC ABI, instruction-timing or complete appliance tests. The
integration fixture uses a short synthetic recipe and simulated time, ADC and
comparator inputs; it does not execute the actual `main()` or complete wash
recipes. Cat-detection and serial hardware drivers remain outside its scope.
Existing host-build warnings and other limits are documented in the test README.

XC8 4.00 was subsequently found under `/opt/cross`, outside PATH. All three
1939 applications now compile, link and pass the resource gate at `-O1`.
After the terminal-editing port, CatGenius uses 13,156/16,384 program words and
458/1,024 RAM bytes, with an estimated 15/16 hardware-stack levels including its
ISR. The 877A CatGenius and GenieDiag builds fail allocation; IOTester fits with
no estimated stack margin.
The [full matrix](pic-build.md) records these results and the compiler profile.
The integration-test expansion changes no production sources or build inputs.
A fresh CatGenius 1939 build passes the same resource gate and produces a HEX
file byte-for-byte identical to the terminal-editing build; it adds no flash,
RAM or hardware-stack usage. The documented 877A limitations remain unresolved.
No image has been flashed. The older MPLAB makefiles still need regeneration;
the standalone build does not depend on them.

Before release, resolve the failed resource gates and inspect compiler call
graphs, the instruction-pointer conversion, Timer4 ISR
latency and shared GPIO instructions. Then carry out controlled board validation,
with mains circuitry enclosed and appropriate isolated test equipment.

## Session handoff

Checkpoint recorded on 11 September 2026: development is on `master-b07`.
The integration framework (`293b4ab`) and its fault scenarios (`f78b613`) are
committed and pushed. The expansion left no pending tracked edits or partially
applied changes. The roadmap above records proposed follow-up work; start with
automatic-mode coverage when implementation is requested.

On resuming, check the branch, worktree and recent commits rather than assuming
this checkpoint is still HEAD. Read this document, the
[test README](../software/tests/README.md), [XC8 report](pic-build.md),
[water specification](water-sensing.md) and relevant
[ccm follow-up entries](ccm-follow-up.md).

Working conventions agreed with Robert:

- Make review- and bisect-friendly commits. Keep bug fixes separate from
  feature/refactoring work, with the regression tests that demonstrate them.
- Use `CatGenius <robert@delien.nl>` for both author and committer.
- Match surrounding indentation and style. Preserve each existing file's
  line endings, including legacy firmware CRLF; use LF for new files. Do not
  make a repository-wide formatting or line-ending conversion.
- XC8 is the supported compiler; do not restore PICC support. After firmware
  changes, run both host compilers and the application/device resource matrix.
  Check flash, RAM and hardware-stack depth, not just whether linking succeeds.
  Keep known target-build failures visible; the reproducible build commands
  and compiler/device-pack versions are in the XC8 report.
- Describe CatGenius behavior on its own terms in firmware comments and
  documentation, without comparisons to other firmware implementations.
- Robert has authorized useful pushes to `master-b07`. That does not authorize
  a release, changes to other branches/tags, or flashing an appliance.
- Leave unrelated files and credentials untouched; do not inspect key
  contents or bulk-stage untracked files.

The host checks can be rerun from the repository root with:

```sh
sh software/tests/run.sh
CC=clang sh software/tests/run.sh
```

The local `documentation/google-group/` and `documentation/project-closure/`
research archives were still untracked at handoff. They have been preserved,
but are not backed up by the pushed branch and will not appear in a fresh
clone. Treat the closure inventory as a historical research snapshot, not the
current B07 status list. Publishing or backing up those archives is separate
work requiring its own review; do not include them incidentally in a commit.

## Commit structure

The initial host-framework commit passes against the B06-based source.
Subsequent bug fixes have their own regression cases. The water-module work is
split into quality decisions, acquisition/interrupt integration, program
preflight, diagnostics, additional edge-case tests and documentation.

The locally archived Google Group and closure-research files are not included
in this implementation series.
