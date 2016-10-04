# FlightLevel
Pebble timer app for GA pilots.
Inspired by the very nice Time flies! app from Graeme Howie.

The app represents a flight as a set of phases and transitions (at the moment preflight -> taxi -> inflight -> taxi -> postflight).

The app main display is divided in three segments:
* Top: Day of month and Zulu time
* Middle: a set of times and duration (flight time, take-off time, landing time...)
* Bottom: simple stopwatch, display the time it was started (minutes only) and the elapsed time.

Controls:
* Up: move to next phase of flight (e.g. move from taxi to in flight just before take-off), a long press cancel the last transition and reverts to the previous phase, in case of error.
* Select: cycle through the tracked times and duration. Long press: display flight time directly (shortcut)
* Down: restart the stopwatch (flyback)
* Back: open the menu. Long press: exit the app

Parts of the code draw heavily on the examples from https://github.com/pebble-examples/ui-patterns
