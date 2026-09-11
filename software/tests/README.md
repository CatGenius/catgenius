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

The initial checks cover water sampling/hysteresis, fill/LED output handling,
stopping all actuators, and dry-program no-ops. Regression cases accompany
subsequent fixes and sensor changes.

New test sources use LF line endings. Existing firmware sources retain CRLF.

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
and controlled hardware validation.
