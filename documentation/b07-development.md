# B07 development: first correctness and water-fault increment

Started on 11 September 2026 from `master-b06` at `e5c34b0`. The historical
`b06` tag stays at `ff3785a`, before the two recent UI fixes. This is development
work, not a release or a claim that an appliance has been validated.

## Implemented in this increment

- Normalize button inputs before storing/comparing one-bit debouncer state.
  This repairs the B06 Setup-button regression on RB5. Normalize the overheat
  input before comparing it with the saved PIC `bit` as well.
  An active heat fault pauses program execution independently of the queued UI
  notification; resume is refused while either heat or ADC failure remains.
- Normalize pause requests and saved actuator states. Pump, dosage and dryer
  getters return port masks, not 0/1, and those masks must not be copied directly
  into one-bit context fields.
  Resume does not re-enable filling if high water was detected during the pause.
- Bound `INS_WAITWATER(0)` using the existing 10-second drain deadline even
  with the pump off. Bound `INS_WAITWATER(1)` using the existing 135-second fill
  deadline even without a live fill request; preserve an already-running fill
  deadline. Timeout immediately pauses actuators and retains the waiting
  instruction. Explicit resume rearms the appropriate deadline.
- Give analog conversions a 10 ms software watchdog. Normal conversions still
  wait for GO/DONE to clear; incomplete conversion data is never accepted.
  Timeout closes the fill-enable output before extinguishing the IR LED
  (unless continuous-LED diagnostics were requested), and schedules another
  attempt at the existing polling interval. It does not automatically refill.
- Expose `water_valid()`, `water_failed()` and the last completed raw reading.
  Startup and timeout recovery require at least eight completed samples and
  reaching a hysteresis endpoint before the debounced level is qualified.
  The old level remains available for inspection while unqualified, but cannot
  satisfy a program water wait or permit dryer startup/restoration.
- Stop an active or paused CatGenius program on an ADC timeout, before further
  instruction execution. All actuators are turned off, the existing execution
  error indication (panel error 4) is raised, and the persisted box state is
  retained. Successful sensor recovery does not automatically restart a wash.
- Pause instead of energizing the dryer when water is high or unqualified.
  Retry the same dryer instruction after an explicit resume. High/unqualified
  water detected during drying pauses the program; a saved running dryer cannot
  be restored until the level is qualified low. This uses the existing drain
  error indication (panel error 2).
- Extend the `water` diagnostic with qualification/fault status, the last raw
  completed reading, fill-enable and IR-LED state, and instantaneous RB3.
  The PIC16F877A reading is explicitly labeled **digital**, not an ADC result.

The ADC fault flag stays set until qualification succeeds. Filling requests
made while it is set are rejected. A new, explicit fill request is needed after
recovery. A low-water indication is **not** proof that all water has drained;
the existing timed drain and wash recipes have not been shortened or changed.

## Deliberately unchanged / still open

- RA1 versus RB3 sensing policy, analog thresholds and dirty-guide detection.
  The measured CatGenius thresholds are retained without substitution.
  The earlier `414` constant remains unused; a raw value by itself is not a
  reliable dirt diagnosis without fill/probe state and other context.
- RD0 probe sequencing and sensor LED settling. `DETECTTIME` still evaluates to
  1 ms; only its erroneous 10 ms comment was corrected. No new RD0 probe pulse
  that might permit water flow was introduced.
- Filtering or calibration changes, analog acquisition accuracy, averaged
  quality diagnosis, timestamped simultaneous RA1/RB3 snapshots and persistent
  fault history. The diagnostic GPIO reads are live and are **not** simultaneous
  with the last completed sensor sample.
- Dosage calibration, 50/60 Hz compensation, cartridge volume/accounting,
  cartridge-type behavior, cat-triggered dose selection and scent policy.
- Other closure findings, including the child-lock long-press path, command
  parser bounds, event-queue overflow, idle overheat/pause behavior, timer/RTC
  corner cases, and a reproducible supported PIC build environment.

The dryer interlock is in the CatGenius interpreter, **not** a universal
low-level relay interlock. GenieDiag and IOTester remain service tools; direct
actuator commands are not converted into an automatically safe wash controller.
Sensor acquisition failure does inhibit filling in the shared water module.

The watchdog and wait deadlines require the main loop and timer to keep running.
They do not solve blocking serial output, a stalled main loop, all possible
stale readings, a stuck sensor value, relay failure or faulty analog circuitry.
There is no claim of a hard real-time cutoff or independent safety certification.

## Verification and next gate

Run [the host regression checks](../software/tests/README.md) with GCC and Clang.
They execute the changed C paths with supplied inputs for both PIC-target
configurations and use address/undefined-behavior sanitizers. They are not a
PIC simulator, target ABI test or whole-appliance regression suite.

On 11 September 2026, both target configurations passed with both GCC and Clang.
The tracked source changes also passed whitespace checks with CRLF recognized,
and existing firmware C/header files retain CRLF. All new test files use LF,
with a test-directory `.gitattributes` enforcing that choice. Existing host-build
warnings are described in the test README rather than being treated as a clean
PIC build.

No PIC compiler was found on the current PATH, and the checked-in MPLAB projects
lack their generated `nbproject/Makefile-impl.mk` and `Makefile-variables.mk`
files; even a dry-run CatGenius build stops at the missing latter file.
No PIC firmware image has been built or flashed during this increment. The next
gate is to regenerate
the project makefiles and build CatGenius, GenieDiag and IOTester for both PICs,
check flash/RAM use and compiler call-graph diagnostics, then perform controlled
hardware validation on the identified board revision. Keep mains circuitry
enclosed and use appropriate isolated test equipment.

## Commit and test structure

The host framework is the first commit and passes against the B06-based source.
Each subsequent fix or sensor change adds its own checks and passes the suite
with GCC and Clang for both PIC configurations. Existing bugs were reproduced
before applying their fixes: button state truncation, repeated heat events,
delayed heat enforcement, lost pause context, fill restoration after high water,
and water waits depending on actuator state. The new dryer guard was also
checked against the preceding behavior.

Bug fixes are separate from the qualification, ADC-watchdog, dryer-interlock
and diagnostic additions. This checkpoint does not incorporate the locally
archived Google Group or closure-research files.

## Further water-sensor development

A coherent water-module implementation would keep three responsibilities
distinct: water level, optical quality and fill permission. This larger change
is **not implemented by this checkpoint**. Comparator probing and compatibility
policy still need agreement.

The proposed design is:

- Use illuminated, RD0-enabled RB3 comparator probes for water level, with
  consecutive-sample change confirmation. A six-sample window and nominal
  100 ms polling interval are starting points for validation. Restore RD0
  and IR illumination according to the active fill request.
- Evaluate optical quality separately using four-reading AN1 means, repeated
  confirmation and distinct fault reasons. A threshold of 410 is a candidate
  to validate, not a measured calibration for this implementation and not a
  replacement for the water-level threshold.
- Give quality faults a separate recovery check requiring four consecutive
  good batches. Define equality explicitly: initial acceptance at or below
  the threshold, recovery strictly below it.
- Keep bounded ADC-completion polling and reject partial conversion results.

These paths need a coordinated non-blocking state machine, explicit ownership
of RD0/IR illumination, and integration with program start, pause, faults and
manual recovery. Poor optical reflection must not be labeled conclusively
as dirt; water and other optical faults can cause it too.

Comparator probing intentionally introduces brief valve-enable pulses that
the current tick-less approach avoids. Pulse duration and output restoration
need deliberate timing design and board validation, including main-loop
delays. This is a behavioral change, not just additional diagnostics.

The first implementation target would be the 16F1939. The 16F877A's grouped
analogue-pin configuration and tighter code budget require separate
investigation; the current digital fallback must not silently be presented as
equivalent. Calibration must be explicit rather than assuming EEPROM contents
have a particular meaning. Quality-fault/UI semantics and physical sampling
accuracy remain to be validated. Real PIC builds and controlled board checks
are required before treating this implementation as release-ready.
