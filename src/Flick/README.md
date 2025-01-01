# Flick

This is a reverb, tremolo, and delay pedal. The original goal of this pedal was to displace the Strymon Flint (Reverb and Tremolo) and the AionFX Elysium (Ambient Delay) to save space on my small pedal board.

### Effects

**Sploodge Reverb:** This is a reverb that adds a few extra parameters to the
ReverbSc reverb and was shamelessly "borrowed" from this project:

https://github.com/wgd-modular/loewenzahnhonig-firmware/tree/main/src/splooge-reverb

**Tremolo:** Sine wave based tremolo meant to mimic tube bias tremolo found
on some Fender amps. It's nice and smooth just like you'd expect.

**Delay:** Standard Daisy Seed digital delay.

### Demo

Here's an early alpha version of how it's working (27 December 2024):

[![Demo Video](https://img.youtube.com/vi/-sD-U93r3Rw/0.jpg)](https://youtu.be/-sD-U93r3Rw)

### Controls

| CONTROL | DESCRIPTION | NOTES |
|-|-|-|
| KNOB 1 | Reverb Dry/Wet Amount |  |
| KNOB 2 | Tremolo Speed |  |
| KNOB 3 | Tremolo Depth |  |
| KNOB 4 | Delay Time |  |
| KNOB 5 | Delay Feedback |  |
| KNOB 6 | Delay Dry/Wet Amount |  |
| SWITCH 1 | Reverb Feedback | **UP** - High<br/>**MIDDLE** - Med<br/>**DOWN** - Low |
| SWITCH 2 | Reverb Tone | **UP** - Bright<br/>**MIDDLE** - Neutral<br/>**DOWN** - Dark |
| SWITCH 3 | Sploodge | **UP** - Shimmer<br/>**MIDDLE** - Subtle<br/>**DOWN** - Off |
| FOOTSWITCH 1 | Reverb On/Off | Long press for DFU mode. |
| FOOTSWITCH 2 | Delay/Tremolo On/Off | Normal press toggles delay.<br/>Double press toggles tremolo.<br/><br/>**LED:**<br/>- 100% when only relay is active<br/>- 40% pulsing when only tremolo is active<br/>- 100% pulsing when both are active |

### Installation

Create a `daisy-seed/` directory on your computer.
```
mkdir daisy-seed
```

Inside of the `daisy-seed/` directory, clone this project:
```
git clone https://github.com/joulupukki/hothouse-effects.git
```

Download the submodules (third-party code that this project depends on): libDaisy and DaisySP.
```
cd hothouse-effects
git submodule update --init --recursive
```

Build libDaisy and DaisySP (this may take a while):
```
make -C libDaisy
make -C DaisySP
```

Once that is done, you should be able to go into the `src/Flick/` directory and build the effect:
```
cd src/Flick/
make
```

To install the effect onto the Hothouse pedal, put your Hothouse into DFU mode and run:
```
make program-dfu
```

**Note:** With any of the Hothouse Example effects installed or the Flick installed onto your Hothouse, you can put the Hothouse into DFU mode by pressing and holding the left footswitch button for 3 seconds. Keep pressing it until the LED lights flash alternatively.

### License

This software is licensed under the GNU General Public License (GPL) for open-source use.
