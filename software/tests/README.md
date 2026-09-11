# Host regression checks

From the repository root:

```sh
sh software/tests/run.sh
CC=clang sh software/tests/run.sh
```

The runner builds and executes the actual `catgenie120.c`, `water.c`,
`litterlanguage.c` and `cmdline_box.c` with supplied GPIO, ADC, EEPROM and timer
inputs, once with `_16F877A` and once with `_16F1939`. It uses AddressSanitizer
and UndefinedBehaviorSanitizer. Its temporary executables are removed on exit.
Leak checking is disabled: the fixture does not allocate heap memory, and
LeakSanitizer cannot run under some debugger/sandbox environments.

New test sources use LF line endings, enforced by the local `.gitattributes`.
Existing firmware sources retain CRLF.

Covered behaviors:

- Both buttons produce normalized, debounced press/release events, including
  Setup on RB5; a steady overheat input produces only one event per transition.
  An active heat fault pauses execution even without processing the UI event.
- Pause/resume restores all 16 combinations of fill/pump/dosage/dryer state,
  both bowl/arm directions, and remaining timers. Repeated nonzero pause
  requests do not overwrite the saved context.
  Filling is not restored if high water arrived during the pause.
- Water level is unqualified at startup, becomes qualified after the initial
  sample window, and retains the existing level thresholds and hysteresis.
- Waiting for low water times out with the drain pump **off**; waiting for high
  water is bounded even with filling off and does not reset an existing fill
  deadline. An unqualified initial low state cannot satisfy a wait.
- The dryer cannot start with high or unqualified water readings. High water
  during drying pauses all actuators, and resume cannot restore the dryer until
  low water is qualified. The blocked instruction is retried, not skipped.
- Dry-program water waits and dryer instructions remain no-ops.
- For the analog configuration, incomplete ADC results are ignored before the
  watchdog expires. Timeout inhibits filling, invalidates the level, preserves
  the last completed sample, and retries without automatically restarting
  filling. Recovery requires a new qualification window.
- An ADC failure stops an active or paused CatGenius program and all actuators,
  reports execution error 4, and preserves the persisted wet-box state. Sensor
  recovery does not restart the program. Resume is refused during the fault,
  including when the saved dryer state was off.
- The water diagnostic distinguishes unqualified, high/low and timeout status,
  analog and digital readings, and preserves the last raw sample after a fault.

## Limits

These are software regression checks, **not PIC firmware builds or appliance
validation**. The mock `bit` is C `_Bool`; actual GPIO mask and one-bit field
operations are exercised, but PIC-specific storage allocation is not modeled.
The timer fixture supplies deterministic time and does not test the real timer
ISR, wraparound implementation or scheduling latency. ADC completion is a
supplied input, not an analog/conversion-timing simulation.

The fixture uses native host pointers to fetch instructions. It deliberately
does not execute the PIC-specific `INS_CALL` pointer conversion, complete ROM
wash recipes, UI event queue or electrical circuits. PIC-address debug logging
is disabled in the host fixture. GCC consequently warns about the existing
PIC-only `memcpy` pointer workaround: host pointers are wider than the source
integer. Existing unused-parameter, fall-through, dangling-else and register
mask conversion warnings are also visible; they are not new regression-test
failures. Register-pointer signedness warnings alone are disabled in the runner.

Both PIC targets still need real compiler/linker checks, code/RAM budget checks
and controlled hardware validation. See the [B07 development notes](../../documentation/b07-development.md).
