# Host regression checks

From the repository root:

```sh
sh software/tests/run.sh
CC=clang sh software/tests/run.sh
```

The runner first tests `waterquality.c` independently with warnings treated as
errors. It then builds and executes the actual `catgenie120.c`, `water.c`,
`waterquality.c`, `litterlanguage.c` and `cmdline_box.c` with supplied GPIO,
ADC, EEPROM and timer inputs, once with `_16F877A` and once with `_16F1939`.
Separate fixtures execute the actual UI, command parser, Timer1 and RTC modules.
An integration executable also links the real board, water, quality, UI,
interpreter, RTC and command-parser modules as separate translation units.
Python 3 tests the PIC resource-report gate without needing the target compiler.
It uses AddressSanitizer
and UndefinedBehaviorSanitizer. Its temporary executables are removed on exit.
Leak checking is disabled: the fixture does not allocate heap memory, and
LeakSanitizer cannot run under some debugger/sandbox environments.

New test sources use LF line endings, enforced by the local `.gitattributes`.
Existing firmware sources retain CRLF.

## Integrated application scenarios

`integration.c` runs one scenario per process, so production static variables
start naturally; no reset hooks or private-state overrides are needed. It runs
the active workers in the order used by `catgenius.c`, with a deterministic
one-millisecond loop and a logical 48-bit timer service. Cat detection and
serial input are inactive; their hardware drivers are not linked.

Button gestures pass through the actual GPIO debouncer and UI. Checks observe
actuator outputs/directions, numbered panel LEDs, water status and EEPROM
writes. Every pass checks that an idle or paused interpreter leaves actuators
off, and that the dryer only runs with qualified low water. RD0 sensing probes
are distinguished from logical fill requests. On failure the fixture prints
the last 64 state changes, their times and workers, plus recent firmware output.

There are 12 scenarios on both configurations and 11 additional analogue-only
scenarios (35 executions per compiler):

- Complete manual wash, scoop-only operation, child lock/unlock, and pausing
  during dosing. The synthetic recipe's 0.2 ml instruction receives exactly two
  seconds of **logical output on-time**, including across a manual or overheat
  pause. This is not a measurement of physical volume or relay/motor response.
- Start release debouncing in the same pass as an overheat assertion; active
  overheat handling; refusal to resume until cooling; no automatic resumption.
- High water arriving during a fill pause or during drying, blocked dryer
  resumption until qualified low water, and explicit successful recovery.
- The 135-second fill deadline and 10-second drain-wait deadline, including
  the next-pass UI response. Stopping a failed wash preserves its wet-box record.
- An invalid recipe instruction while dosing: the real deferred UI handler
  stops the interpreter and every actuator on its following pass.
- Startup cleanup with a supplied wet-box EEPROM record. This does not yet
  simulate a power cut and reboot with the preceding run's EEPROM contents.
- Paused preflight completion; optical faults during running/paused preflight;
  latched level faults; and the five-second preflight deadline when comparator
  samples keep changing despite acceptable optical quality.
- ADC/probe timeouts during preflight and active/paused execution. Recovery
  qualifies fresh readings but never restarts the program; a new request can
  subsequently complete. Failed preflight leaves the tidy-box record untouched.

The short synthetic recipe exercises all six actuators without PIC-only calls.
Supplied sensor inputs go high after two seconds of filling and low after two
seconds of draining. Analogue builds receive explicit ADC completions and
nominal 500-microsecond Timer4 events between main-loop passes, invoking the
real `water_isr()`. These are deterministic test inputs, not an electrical,
hydraulic, interrupt-latency or instruction-cycle simulator. The separate
Timer1 tests remain responsible for the real timer driver. The integration
fixture does not execute `main()`, its initialization of all peripherals, or
the complete interrupt dispatcher.

Next useful extensions are cat-departure and automatic-mode scheduling, complete
recipe traversal including target-aware call addresses, power-cut/reboot
scenarios retaining EEPROM, and real serial/cat-sensor driver fault tests.
The prioritized checklist and working conventions are maintained in the
[B07 roadmap and handoff](../../documentation/b07-development.md#roadmap).

## Module regression checks

Covered behaviors:

- Locked short/long single-button gestures cannot start, stop or pause a wash;
  the two-button unlock gesture remains available. The UI fixture uses a stub
  interpreter; the separate integration scenarios connect it to the real one.
- Fault bursts retain assertions and final clears, normalize nonzero arguments,
  and defer handler-generated events. Tests cover simultaneous types and both
  assertion/clear orders. They do not specify a new acknowledgement policy.
- Command argument boundaries, empty/whitespace-only input, invalid commands
  and streamed lines. Backspace (0x08) and Delete (0x7f) both remove the last
  character and echo Backspace, space, Backspace to erase it on the terminal.
  Tests capture the exact output for empty/full buffers, complete-line deletion,
  edits across worker calls, mixed erase keys and echo-off operation. CRLF input
  and ENQ/ACK behavior remain covered. Dispatch occurs after character handling
  returns, reducing command call depth.
- Idle pause is a no-op, hot starts/cleanup are refused, and a new request after
  cooling can execute. Active-program heat protection remains covered too.
- Timer carry, saturation, disabled deadlines, register-read rollovers, pending
  interrupt flags and exactly-once overflow accounting after ISR service.
  RTC midnight/week rollover, delayed catch-up and all 24 hour-setting inputs.
- Both buttons produce normalized, debounced press/release events, including
  Setup on RB5; a steady overheat input produces only one event per transition.
  An active heat fault pauses execution even without processing the UI event.
- Pause/resume restores all 16 combinations of fill/pump/dosage/dryer state,
  both bowl/arm directions, and remaining timers. Repeated nonzero pause
  requests do not overwrite the saved context.
  Filling is not restored if high water arrived during the pause.
- Water level is unqualified at startup. The 16F877A retains its digital RA1
  sampling and eight-step hysteresis. The 16F1939 qualifies six consecutive
  comparator samples independently of the ADC mean.
- Waiting for low water times out with the drain pump **off**; waiting for high
  water is bounded even with filling off and does not reset an existing fill
  deadline. An unqualified initial low state cannot satisfy a wait.
- The dryer cannot start with high or unqualified water readings. High water
  during drying pauses all actuators, and resume cannot restore the dryer until
  low water is qualified. The blocked instruction is retried, not skipped.
- Dry-program water waits and dryer instructions remain no-ops.
- For the analogue configuration, four completed ADC reads produce an integer
  mean, with illumination/acquisition delays and no partial-batch publication.
  Fill changes cancel acquisition rather than mixing contexts.
- Timer4 ends a non-filling probe and restores outputs without main-loop work.
  Filling and diagnostic-LED restoration, cancellation with a pending interrupt,
  and preservation of the global interrupt state are covered.
- ADC and probe timeouts inhibit filling, invalidate the level and preserve
  the last complete measurement. Recovery requires a fresh qualification
  window and does not automatically restore a fill request.
- An ADC or probe failure stops an active or paused CatGenius program and all actuators,
  reports execution error 4, and preserves the persisted wet-box state. Sensor
  recovery does not restart the program. Resume is refused during the fault,
  including when the saved dryer state was off.
- Optical-quality decisions cover all 121 combinations of repeat-window bad
  counts, first-pass acceptance, comparator precedence, analogue-only checks,
  history margins at different thresholds and the saved last mean.
- Optical recovery requires four strictly good consecutive batches. New check
  requests cannot bypass it; ADC failure breaks recovery evidence. A latched
  comparator/level fault requires a new check, including after an acquisition
  failure has occurred as well.
- Full-wash preflight waits for both quality and fresh low water, with bounded
  waiting, pause/resume, cancellation and failure handling before any actuator
  instruction. Scoop-only, recovery cleanup and 16F877A startup remain available.
  Failed/cancelled preflight preserves a tidy-box record as well as an existing
  wet-box record; successful startup still records program activity.
- Diagnostics distinguish quality and acquisition fault reasons, unavailable
  initial measurements, saved mean/fill context and live RD0 during a probe.
  The water command remains read-only.

## Limits

These are software regression checks, **not PIC firmware builds or appliance
validation**. The mock `__bit` is C `_Bool`; actual GPIO mask and one-bit field
operations are exercised, but PIC-specific storage allocation is not modeled.
The interpreter fixture supplies deterministic time. The separate timer fixture
executes the real timer code using a test-only mapping of `long` to a 32-bit host
type and packed six-byte timers. It models selected register/ISR interleavings,
not PIC byte-access timing, every possible interrupt race or scheduling latency.
The actual `water_isr()`
body is exercised, but Timer4 expiry, ADC results/completion and comparator
levels are supplied inputs. This does not simulate analogue behavior, peripheral
clocking, generated interrupt dispatch or worst-case interrupt latency.

The fixture uses native host pointers to fetch instructions. It deliberately
does not execute the PIC-specific `INS_CALL` pointer conversion, complete ROM
wash recipes or electrical circuits. PIC-address debug logging
is disabled in the host fixture. GCC consequently warns about the existing
PIC-only `memcpy` pointer workaround: host pointers are wider than the source
integer. Existing unused-parameter, fall-through, dangling-else and register
mask conversion warnings are also visible; they are not new regression-test
failures. Register-pointer signedness warnings alone are disabled in the runner.

Real compiler/linker results and unresolved 877A resource limits are recorded in
the [XC8 build report](../../documentation/pic-build.md). Both targets still need
controlled hardware validation. See the [B07 development notes](../../documentation/b07-development.md)
and [water-sensing specification](../../documentation/water-sensing.md).
