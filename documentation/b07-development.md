# B07 development

Started on 11 September 2026 from `master-b06` at `e5c34b0`. The historical
`b06` tag stays at `ff3785a`, before the two recent UI fixes. This is development
work, not a release or a claim that an appliance has been validated.

## Implemented

The first checkpoint fixed button/overheat normalization, lost pause context,
fill restoration after high water arrived while paused, and unbounded water
waits. These fixes remain separate commits with their regression tests.

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
- Other closure findings, including child-lock long-press handling, command
  parser bounds, event-queue overflow, idle overheat/pause behavior, timer/RTC
  corner cases and a reproducible supported PIC build environment.

## Verification and next gate

Run [the host regression checks](../software/tests/README.md) with GCC and Clang.
The quality-decision tests cover every combination of 0..10 analogue and
comparator failures in the repeat window, history boundaries and recovery.
The firmware tests exercise actual changed C paths with supplied inputs for
both PIC configurations, including the Timer4 handler and wash preflight.

On 11 September 2026, every new commit passed the checks with both compilers,
using address/undefined-behavior sanitizers. The quality-decision module also
passes a strict C90 syntax check. Both edited MPLAB configuration XML files
parse successfully. Whitespace checks recognize CRLF: existing firmware and
project files retain their line endings; new source, test and documentation
files use LF. Test-directory attributes enforce LF for the framework.

These are not PIC ABI, instruction-timing or complete appliance tests. The
fixture does not execute complete wash recipes or the UI event queue. Existing
host-build warnings and other limits are documented in the test README.

No PIC compiler was found on the current PATH. The MPLAB projects still lack
generated `nbproject/Makefile-impl.mk` and `Makefile-variables.mk` files; a
CatGenius dry-run build fails at the missing latter file. No PIC image has been
built or flashed during this work.

Before release, regenerate the project makefiles and build CatGenius, GenieDiag
and IOTester for both PICs. Check flash/RAM, compiler call graphs, Timer4 ISR
latency and shared GPIO instructions. Then carry out controlled board validation,
with mains circuitry enclosed and appropriate isolated test equipment.

## Commit structure

The initial host-framework commit passes against the B06-based source.
Subsequent bug fixes have their own regression cases. The water-module work is
split into quality decisions, acquisition/interrupt integration, program
preflight, diagnostics, additional edge-case tests and documentation.

The locally archived Google Group and closure-research files are not included
in this implementation series.
