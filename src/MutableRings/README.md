# Mutable Rings (IN PROGRESS)

TODO

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
| FOOTSWITCH 1 | - | Long press for DFU mode (TODO: NOT WORKING). |
| FOOTSWITCH 2 | Reverb On/Off |  |

### Installation

Run this to create the Makefile:
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
```
(DEBUG is too big to fit)

Run this to build the code:
```
cmake --build build
```

Put the Hothouse into DFU mode and use this to install the .bin:
```
make program-dfu
```

### License

This software is licensed under the GNU General Public License (GPL) for open-source use.
