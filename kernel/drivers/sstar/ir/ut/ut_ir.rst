======================================================
ut_ir.rst is the guide to use sigmastar ut_ir demo
======================================================
How to get the ut_ir:
Just use "Make" command, it will compile the ut_ir.c into ut_ir.

How to use the ut_ir:
(1) run ut_ir
format  : "./ut_ir [group] [event] [decode_mode]"
example : ./ut_ir 0 event0 1
[group] : ir group id.
[event] : input event id
[decode_mode]: 1->Full, 2->Raw, 3->RC5, 4->S/W.
press the remote control button and confirm the key value.
(2) stop ut_ir
ctrl + c
