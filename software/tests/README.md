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
It uses AddressSanitizer
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
validation**. The mock `bit` is C `_Bool`; actual GPIO mask and one-bit field
operations are exercised, but PIC-specific storage allocation is not modeled.
The timer fixture supplies deterministic time and does not test the real timer
ISR, wraparound implementation or scheduling latency. The actual `water_isr()`
body is exercised, but Timer4 expiry, ADC results/completion and comparator
levels are supplied inputs. This does not simulate analogue behavior, peripheral
clocking, generated interrupt dispatch or worst-case interrupt latency.

The fixture uses native host pointers to fetch instructions. It deliberately
does not execute the PIC-specific `INS_CALL` pointer conversion, complete ROM
wash recipes, UI event queue or electrical circuits. PIC-address debug logging
is disabled in the host fixture. GCC consequently warns about the existing
PIC-only `memcpy` pointer workaround: host pointers are wider than the source
integer. Existing unused-parameter, fall-through, dangling-else and register
mask conversion warnings are also visible; they are not new regression-test
failures. Register-pointer signedness warnings alone are disabled in the runner.

Both PIC targets still need real compiler/linker checks, code/RAM budget checks
and controlled hardware validation. See the [B07 development notes](../../documentation/b07-development.md)
and [water-sensing specification](../../documentation/water-sensing.md).
