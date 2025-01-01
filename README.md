# Hothouse Effects

This is my own personal collection of effects built for the [Hothouse DSP Pedal](https://clevelandmusicco.com/hothouse-diy-digital-signal-processing-platform-kit/). Additional example effects can be found in their [Hothouse Exmaples](https://github.com/clevelandmusicco/HothouseExamples/) repo.

### Effects

[Flick](src/Flick/): A reverb, tremolo, and delay pedal in one.

[Platerra](src/Platerra/): A reverb based on the Dattorro reverb.

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

Once that is done, you should be able to go into one of the effects subdirectories (`src/xyz`) and run `make`.

To install the effect onto the Hothouse pedal, put your Hothouse into DFU mode and run:
```
make program-dfu
```

**Note:** With any of the Hothouse Example effects installed or the Platerra installed onto your Hothouse, you can put the Hothouse into DFU mode by pressing and holding the left footswitch button for 3 seconds. Keep pressing it until the LED lights flash alternatively.

### License

This software is licensed under the GNU General Public License (GPL) for open-source use.