okay

this is a working example of receive and send processes with a quite complex pattern

simplified and unified the code
moved to blocking in and out
2 processes, 1 way

so - this now has:
- two i/o procs in deadlock
- same thread priority

not sure why
- not fixed by changing thread priority


