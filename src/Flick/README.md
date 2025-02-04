# Flick

This is a reverb, tremolo, and delay pedal. The original goal of this pedal was to displace the Strymon Flint (Reverb and Tremolo) and the AionFX Elysium (Ambient Delay) to save space on my small pedal board.

### Effects

**Platerra Reverb:** This is a plate reverb based on the Dattorro reverb. It's also available in this project in the [src/Platerra/](src/Platerra/) subdirectory.

**Tremolo:** Sine wave based tremolo meant to mimic tube bias tremolo found
on some Fender amps. It's nice and smooth just like you'd expect.

**Delay:** Standard Daisy Seed digital delay.

### Demo

Updated demo video (6 January 2025):

[![Demo Video](https://img.youtube.com/vi/RR4Hccq0VbE/0.jpg)](https://www.youtube.com/watch?v=RR4Hccq0VbE)

Here's an early alpha version of how it's working (27 December 2024):

[![Demo Video](https://img.youtube.com/vi/-sD-U93r3Rw/0.jpg)](https://youtu.be/-sD-U93r3Rw)

### Controls (Normal Mode)

| CONTROL | DESCRIPTION | NOTES |
|-|-|-|
| KNOB 1 | Reverb Dry/Wet Amount |  |
| KNOB 2 | Tremolo Speed |  |
| KNOB 3 | Tremolo Depth |  |
| KNOB 4 | Delay Time |  |
| KNOB 5 | Delay Feedback |  |
| KNOB 6 | Delay Dry/Wet Amount |  |
| SWITCH 1 | Reverb knob funcion | **UP** - 0% Dry, 0-100% Wet<br/>**MIDDLE** - Dry/Wet Mix<br/>**DOWN** - 100% Dry, 0-100% Wet |
| SWITCH 2 | Tremolo Waveform | **UP** - Square<br/>**MIDDLE** - Triangle<br/>**DOWN** - Sine<br/>*Square wave currently clicks and this is a [known bug](https://github.com/joulupukki/hothouse-effects/issues/9).* |
| SWITCH 3 | Trem & Delay Makeup Gain | **UP** - Plus<br/>**MIDDLE** - Normal<br/>**DOWN** - None |
| FOOTSWITCH 1 | Reverb On/Off | Normal press toggles reverb on/off.<br/>Double press toggles reverb edit mode (see below).<br/>Long press for DFU mode. |
| FOOTSWITCH 2 | Delay/Tremolo On/Off | Normal press toggles delay.<br/>Double press toggles tremolo.<br/><br/>**LED:**<br/>- 100% when only relay is active<br/>- 40% pulsing when only tremolo is active<br/>- 100% pulsing when both are active |

### Controls (Reverb Edit Mode)
*Both LEDs flash when in edit mode.*

| CONTROL | DESCRIPTION | NOTES |
|-|-|-|
| KNOB 1 | Reverb Amount (Wet) | Not saved. Just here for convenience. |
| KNOB 2 | Pre Delay | 0 for Off, up to 0.25 |
| KNOB 3 | Decay |  |
| KNOB 4 | Tank Diffusion |  |
| KNOB 5 | Input High Cutoff Frequency |  |
| KNOB 6 | Tank High Cutoff Frequency |  |
| SWITCH 1 | Tank Mod Speed | **UP** - High<br/>**MIDDLE** - Medium<br/>**DOWN** - Low |
| SWITCH 2 | Tank Mod Depth | **UP** - High<br/>**MIDDLE** - Medium<br/>**DOWN** - Low |
| SWITCH 3 | Tank Mod Shape | **UP** - High<br/>**MIDDLE** - Medium<br/>**DOWN** - Low |
| FOOTSWITCH 1 | Save & Exit | Saves all parameters and exits Reverb Edit Mode.<br/>Long press for DFU mode. |
| FOOTSWITCH 2 | Save & Exit | Saves all parameters and exits Reverb Edit Mode. |

### Factory Reset (Restore default reverb parameters)

To enter factory reset mode, **press and hold** **Footswitch #2** when powering the pedal. The LED lights will alternatively blink slowly.

1. Rotate Knob #1 to 100%. The LEDs will quickly flash simultaneously and start blinking faster.
2. Rotate Knob #1 to 0%. The LEDs will quickly flash simultaneously and start blinking faster.
3. Rotate Knob #1 to 100%. The LEDs will quickly flash simultaneously and start blinking faster.
4. Rotate Knob #1 to 0%. The LEDs will quickly flash simultaneously, defaults will be restored, and the pedal will resume normal pedal mode.

To exit factory reset mode without resetting. Power off the pedal and power it back on.

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
