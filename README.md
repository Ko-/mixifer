# Mixifer
This repository contains two reference implementations in C and one optimized ARM Cortex-M3/M4 implementation of the Mixifer permutation. Its design rationale is specified in [this academic research paper](https://tosc.iacr.org/index.php/ToSC/article/view/847/799).

## Compiling and running the C code
Just type `make`.

## Compiling and running the ARM Cortex-M code
The build system is currently producing binaries for the STM32L100C and STM32F407 development boards. There are a number of software dependencies here:

  * The [arm-none-eabi toolchain](https://launchpad.net/gcc-arm-embedded) toolchain for cross-compiling. On most Linux systems, the correct toolchain gets installed when you install the `arm-none-eabi-gcc` (or `gcc-arm-none-eabi`) package.
  * The [libopencm3](https://github.com/libopencm3/libopencm3) firmware library, for making it nicer to work with the development boards. The Makefile should be updated to point to the right directory where libopencm3 resides.
  * The [stlink](https://github.com/texane/stlink) tools for flashing the code on the board. This gets called by `deploy.sh`. `stlink` might be in your package repository.
  * The host-side Python code requires the [pyserial](https://github.com/pyserial/pyserial) module. Your package repository might offer `python-serial` or `python-pyserial` directly. Alternatively, this can be easily installed from PyPA by calling `pip install pyserial` (or `pip3`, depending on your system). If you do not have `pip` installed yet, you can typically find it as `python3-pip` using your package manager.

Now run `make`. The current code writes output over serial with 115200 baud. It expects `TX` is connect to `PA3` and `RX` to `PA2`. Use `host.py` to keep listening to your serial device (currently `/dev/ttyUSB0`, change if yours is different). Then use `deploy.sh` to flash the code. It should automatically start executing.

## Troubleshooting
[This GitHub repository](https://github.com/joostrijneveld/STM32-getting-started) contains some more details in case something goes wrong.
