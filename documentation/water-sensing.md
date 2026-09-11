# Water sensing and wash preflight

The 16F1939 implementation keeps water level, optical quality and permission to
fill separate. These are development changes: host tests pass, but no target
binary or physical appliance has been validated with this implementation.

## Signals and acquisition

| Operation | RD0: valve enable | RB2: IR illumination | Measurement |
| --- | --- | --- | --- |
| Normal sampling, not filling | Low during ADC; briefly high for comparator probe | On while sampling | Four-read AN1 mean, then RB3 |
| Normal sampling, filling | High | On continuously | Four-read AN1 mean, then RB3 |
| Quality check with comparator | Low during ADC; briefly high for each probe | On through confirmation | Four-read AN1 mean and RB3 |
| Analogue-only quality check | Low throughout the check | On through confirmation | Four-read AN1 mean |
| Optical-fault recovery | Low throughout recovery | On while sampling | Four-read AN1 mean |
| Latched level/comparator quality fault | Low until a new check is requested | On while sampling | Means for inspection; no automatic comparator retry |

RD0 high is permission to fill, not proof that the relay or water supply is on.
The comparator can inhibit the valve while RD0 remains high. Conversely, RD0
low changes the comparator circuit's operating state, so an arbitrary live RB3
read is not an enabled water-level measurement.

Normal cycles are scheduled 100 ms apart from their start. Each cycle gives
illumination 25 ms to settle, then takes four completed AN1 conversions. ADC
results are right-justified, referenced to VDD/VSS, with an Fosc/32 clock.
At 4 MHz this is an 8 microsecond ADC clock. Inter-conversion acquisition is
at least 25 microseconds, rounded up to four Timer1 ticks (32 microseconds at
4 MHz). The cooperative worker may lengthen these intervals; it does not busy
wait or explicitly terminate a conversion to obtain its result.

The reported mean is `floor((a + b + c + d) / 4)` in the range 0..1023. The
largest sum is 4092, within a 16-bit unsigned integer. The fill request is
recorded with the mean. Changing fill state cancels a partial ADC/probe cycle,
so a mean cannot mix samples from different fill contexts. Publication waits
for the complete cycle, including its comparator probe when one is required.
Cancelled or timed-out cycles preserve the previous published measurement.

### Comparator probes and interrupt ownership

Timer4 is dedicated to the probe on the 16F1939; Timer2 remains with the cat
sensor. At 4 MHz, Timer4 uses prescale 4, postscale 1 and PR4 = 124 for a
nominal 0.5 ms period. The register sequence briefly masks global interrupts
while raising RD0 and starting the timer, restoring the previous interrupt
enable state immediately afterward.

The Timer4 handler reads RB3, restores RD0 according to the fill request, and
restores illumination before publishing completion. It performs no callbacks,
timer-helper calls or serial output. CatGenius and GenieDiag service this source
before their other interrupt sources. A delayed main-loop iteration therefore
does not itself extend a completed non-filling probe.

This is an **interrupt-ended pulse**, not a hardware-autonomous cutoff. Actual
pulse width includes interrupt latency and instructions around the timer.
Disabled interrupts, a stalled CPU, register corruption or a failed output
driver are not bounded by this interrupt-based scheme. A missing completion
also has a 10 ms main-loop watchdog, which necessarily depends on the main loop
and Timer1 continuing to run. Validate worst-case latency and shared-latch
instructions in the target build before appliance testing.

Register definitions and peripheral timing are documented in the repository's
[PIC16F1939 datasheet](catgenie120/electronics/components/PIC16F1939.pdf),
particularly the ADC and Timer2/4/6 chapters.

RB3 high during an enabled, illuminated probe means no high-water indication;
RB3 low means high water or another condition affecting the comparator path.
Six consecutive equal normal samples qualify the initial level. Six consecutive
opposite samples change an established level; intervening opposite readings
restart confirmation. Quality-check probes are not counted as regularly spaced
level samples. A high ADC mean alone never changes the water-level state.

## Optical-quality decisions

The default threshold is 410. It is a configurable build value,
`WATER_QUALITY_THRESHOLD`, not measured calibration for every board. The allowed
range is 206..1023 so the 205-count history margin cannot underflow. No EEPROM
addresses or layout are changed or implicitly interpreted as calibration.
The old 414 observation describes one dirty/dry reading, not a level threshold.

For a requested check, the initial mean passes at or below the threshold if
its comparator result is acceptable, or comparator probing was not requested.
Otherwise ten additional batches confirm the outcome:

1. Six or more low comparator results in these ten batches produce the
   level/comparator quality fault. This takes precedence over optical failure.
2. Otherwise, six or more means above the threshold produce an optical fault.
   The **last** mean is saved for subsequent history-dependent decisions.
3. Otherwise, a previously saved mean at or above the threshold and a final
   mean at or above `threshold - 205` produce an optical fault.
4. Otherwise the check passes.

The initial failing batch starts confirmation; it is not included in the ten
repeat counts. Analogue-only checking ignores comparator results. The first
passing path takes four ADC conversions; a complete confirmation takes 44.

An optical fault keeps filling inhibited and automatically samples new means
without valve-enable pulses. Four consecutive means **strictly below** the
threshold clear that quality fault. Equality or a failed acquisition resets
the good-batch counter. A new check request cannot bypass this recovery rule.
A level/comparator quality fault instead needs a new check request; good ADC
means alone cannot clear it. Poor reflection is not conclusively a dirt
diagnosis: water, alignment, illumination and electrical faults can affect it.

Ordinary measurements during washing are available for inspection but do not
apply the empty-box quality test. In particular, submerged optics must not be
reported as an optical-quality fault simply because their mean is high.

## Acquisition failures and washing programs

ADC completion and probe completion each have a 10 ms software watchdog.
Timeout invalidates the level, inhibits filling before extinguishing the LED,
discards incomplete evidence and retries acquisition. A forced diagnostic LED
may remain on, but it does not restore fill permission. Six qualified normal
cycles clear an acquisition fault; that does not restore an earlier fill
request or restart a stopped program.

A full 16F1939 wash validates its program header, then requests a comparator-
enabled quality check. Before the first actuator instruction, both the quality
check and a fresh qualified low-water indication must pass. The whole preflight
has a five-second software deadline. Pausing saves the remaining deadline;
sensor work continues, but the program cannot advance while paused.

A failed preflight stops the request, retains the persisted box state and
reports existing panel error 4. Serial output and the `water` diagnostic give
the specific reason. Quality failure also stops a request that was paused.
The tidy-to-messy EEPROM update is deferred until preflight passes, so a failed
or cancelled check does not schedule an unnecessary cleanup after a reboot.
Cancellation disables a pending probe and abandons an unfinished check without
clearing an already established optical/level fault. Successful sensor recovery
does not resume the stopped request; a new wash request must pass preflight.

A wash requested while acquisition is already faulted can initiate a recovery
check, but does not start a program. Once recovery is complete, request a new
wash. This also permits retry after combined acquisition and level faults.
Optical faults still require their consecutive-good recovery evidence.

Scoop-only programs do not need this preflight. Recovery cleanup deliberately
bypasses the empty-box quality check so it can drain water left by an interrupted
wash. Existing qualified-low-water dryer guards and fill/drain deadlines remain
in force; low at the sensor is not proof that all water has been removed, and
the timed drain recipes have not been shortened.

## Diagnostics and API

The existing `water` command remains read-only. It reports the qualified level,
quality state and threshold, ADC/probe timeout reason, last complete mean with
fill context, and last enabled comparator reading. A measurement that has never
completed is labeled unavailable. Live RD0, IR and RB3 reads are explicitly
separate from those saved readings. They are not simultaneous measurements,
and an analogue-only batch need not have a corresponding comparator sample.

The shared module exposes `water_check(probe)`, `water_check_cancel()`, quality
and acquisition-fault getters, and saved-reading availability/context getters
on analogue configurations. Check requests require stopped actuators. A request
made during another check or an optical fault is refused. Fill requests during
checking, quality failure or acquisition failure are refused. The low-level
actuator service interfaces are not a universal appliance-safety interlock.

## Compatibility and validation

The 16F877A retains its digital RA1 sensing: 1 ms illumination settling,
250 ms polling, eight-step hysteresis/qualification, no Timer4 probes, no
analogue quality check and unchanged program startup. Its diagnostic reading
is labeled digital. It is not presented as equivalent to 16F1939 acquisition.
IOTester does not use the water module and remains a raw service application.

The [host checks](../software/tests/README.md) cover quality arithmetic, probe
ISR behavior, cancellation, fault recovery, preflight and both configurations.
They supply ADC and interrupt completion; they do not model the circuit, real
interrupt latency, PIC ABI or complete washing recipes.

Before release, regenerate the MPLAB makefiles, build both application targets
for both PICs, inspect flash/RAM and call-graph results, and inspect Timer4 and
GPIO instructions. Then validate clean/dry, wet, submerged, dirty/misaligned and
faulted sensor conditions on identified hardware. Measure settling, threshold
margin, probe width under interrupt/serial load, and whether brief enables
cause relay activity or water flow. Keep mains circuitry enclosed and use
appropriate isolated test equipment. Passing host tests is not a safety
certification or authorization to flash an unvalidated appliance.
