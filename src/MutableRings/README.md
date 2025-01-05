# Mutable Rings (IN PROGRESS)

This pedal is based on the work of the [DaisyVerb](https://github.com/stellar-aria/daisyverb) project.

### Demo

TODO

### Controls

| CONTROL | DESCRIPTION | NOTES |
|-|-|-|
| KNOB 1 | Strength |  |
| KNOB 2 | Time |  |
| KNOB 3 | Shape |  |
| KNOB 4 | - |  |
| KNOB 5 | - |  |
| KNOB 6 | - |  |
| SWITCH 1 | Reverb Type | **UP** - Mutable Rings<br/>**MIDDLE** - Dattorro<br/>**DOWN** - All Pass|
| SWITCH 2 | - | **UP** -<br/>**MIDDLE** -<br/>**DOWN** - |
| SWITCH 3 | - | **UP** -<br/>**MIDDLE** -<br/>**DOWN** - |
| FOOTSWITCH 1 | - | Long press for DFU mode. |
| FOOTSWITCH 2 | Reverb On/Off |  |

### Installation

This uses version 14.2 of the [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Download the `arm-none-eabi` .tar.xz file for your host operating system. For example, on my Silicon-based Mac, I chose `arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi.tar.xz`. Unzip it and add the included `bin/` directory to your PATH environment.

Run this to create the Makefile:
```
cmake -B build
```

Run this to build the code:
```
cmake --build build
```

Put the Hothouse into DFU mode and use this to install the .bin:
```
make program-dfu
```

### License

This software is licensed under the GNU General Public License (GPL) for open-source use. The DaisyVerb project is licensed under the MIT license and that includes all of the code inside of the `MutableRings/include/` directory.
