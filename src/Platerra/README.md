# Platerra

This is an implementation of the Dattorro plate reverb adapted from the work originally done for the [Campestria Versio](https://github.com/digitalartifactmusic/PlateauNEVersio) reverb which was originally written by Dale Johnson for [VCV Rack](https://github.com/ValleyAudio/ValleyRackFree/tree/main/src/Plateau).

### Demo

TODO: Build a demo video and show it here.

### Controls

| CONTROL | DESCRIPTION | NOTES |
|-|-|-|
| KNOB 1 | Volume (Dry) |  |
| KNOB 2 | Reverb Amount (Wet) |  |
| KNOB 3 | Reverb Decay |  |
| KNOB 4 | Tank Diffusion |  |
| KNOB 5 | Input High Cutoff Frequency |  |
| KNOB 6 | Tank High Cutoff Frequency |  |
| SWITCH 1 | Tank Mod Speed | **UP** - High<br/>**MIDDLE** - Medium<br/>**DOWN** - Off |
| SWITCH 2 | Tank Mod Depth | **UP** - High<br/>**MIDDLE** - Medium<br/>**DOWN** - Off |
| SWITCH 3 | Pre Delay | **UP** - 0.10<br/>**MIDDLE** - 0.05<br/>**DOWN** - Off |
| FOOTSWITCH 1 | Toggles "100% Wet" mode | Defeats the dry signal knob (sets the dry signal to 0%).<br/><br/>Long press for DFU mode. |
| FOOTSWITCH 2 | Reverb On/Off |  |

### Installation

Create a `daisy-seed/` directory on your computer.
```
mkdir daisy-seed
```

Inside of the `daisy-seed/` directory, clone this project:
```
git clone https://github.com/joulupukki/hothouse-effects.git
```

Download the submodules (third-party code that this project depends on): libDaisy, DaisySP, and PlateauNEVersio.
```
cd hothouse-effects
git submodule update --init --recursive
```

Build libDaisy and DaisySP (this may take a while):
```
make -C libDaisy
make -C DaisySP
```

Once that is done, you should be able to go into the `src/Platerra/` directory and build the effect:
```
cd src/Platerra/
make
```

To install the effect onto the Hothouse pedal, put your Hothouse into DFU mode and run:
```
make program-dfu
```

**Note:** With any of the Hothouse Example effects installed or the Platerra installed onto your Hothouse, you can put the Hothouse into DFU mode by pressing and holding the left footswitch button for 3 seconds. Keep pressing it until the LED lights flash alternatively.

### Dattorro Paper

Lots of good information about the algorithm is available [here](https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf) (PDF).

### License

This software is licensed under the GNU General Public License (GPL) for open-source use.
