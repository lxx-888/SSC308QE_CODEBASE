======================================================
wdt_ut.rst is the guide to use sigmastar wdt_ut demo
======================================================
How to get wdt_ut:
Just use "make" command, it will generate wdt_ut.

How to use wdt_ut:
(1) Start wdt_ut
./wdt_ut start [timeout(s)]
start watchdog, it will keep watchdog alive, the default timeout value is 5s.
(2) Stop wdt_ut
ctrl + c
(3) Reset the chip
./wdt_ut reset [timeout(s)]
start watchdog, but not keep it alive, it will reset the chip after timeout.
