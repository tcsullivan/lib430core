# lib430core

`lib430core` provides the ability to execute binaries built for the MSP430 architecture on non-MSP430 hardware. This library only aims to reliably support the MSP430's CPU or instruction set: this is *not* meant to become a complete virtualization of MSP430 chips with peripheral support.

The intended purpose of this library is to allow for a kind of "sandboxing", or process isolation. The host has complete control over the state and 16-bit memory space of a `lib430core` process, and can memory-map custom interfaces, control execution state and speed, and manage process memory at run time (e.g. paging).

## Building

To build `lib430core`, simply run `make`. The top of the Makefile can be edited to change the toolchain and compiler flags used.

`make` produces a `lib430core.a` which may be linked into other code.

Tests are in the `test` directory, which has its own Makefile.
