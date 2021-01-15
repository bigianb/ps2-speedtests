# ps2-speedtests
Micro profile runs for PS2 emulation

These tests rely on a patched version of the emulator under test to get high resolution timing information from the host.
Using the system timers is no good because they will give us emulated time and we need host time.

A simple scheme is used which wires up clock to 2 memory addresses:

| Address    | Value |
| ---------- | ----- |
| 0x10001900 | returns the current clock() value |
| 0x10001904 | returns CLOCKS_PER_SEC |

For Play! the following branch can be used: https://github.com/bigianb/Play-/tree/clock
For PCSX2 ... TODO

## Setup
Refer to [setup.md](setup.md) for details on how to configure a suitable compilation environment.

## Running

Run make to generate elf files in the build directory. Use the 'run elf' option in the emulator and look at stdout.
