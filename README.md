# FlightLevel
Pebble timer app for GA pilots

The app is divided in three segments:
* Top: Day of month and Zulu time
* Middle: a set of times and duration (flight time, take-off time, landing time...). A press on the select button cycles
through the available information.
* Bottom: simple stopwatch, display the time it was started (minutes only) and the elapsed time. A press on the bottom 
button restarts the stopwatch.

The app represents a flight as a set of phases and transitions (at the moment preflight -> inflight -> postflight with 
take-off and landing as transitions). The top button allows to move to the next phase (simple press) or go back to the
previous phase (long press) to correct an error.