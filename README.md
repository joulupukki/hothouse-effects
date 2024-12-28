# Flick

Contributed by Boyd Timothy \<<btimothy@gmail.com>\>

## Description

This is a reverb, tremolo, and delay pedal. The original goal of this pedal was to displace the Strymon Flint (Reverb and Tremolo) and the AionFX Elysium (Ambient Delay) to save space on my small pedal board.

### Effects

**Sploodge Reverb:** This is a reverb that adds a few extra parameters to the
ReverbSc reverb and was shamelessly "borrowed" from this project:

https://github.com/wgd-modular/loewenzahnhonig-firmware/tree/main/src/splooge-reverb

**Tremolo:** Sine wave based tremolo meant to mimic tube bias tremolo found
on some Fender amps. It's nice and smooth just like you'd expect.

**Delay:** Standard Daisy Seed digital delay.

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

First clone the HothouseExamples project (https://github.com/clevelandmusicco/HothouseExamples/tree/main). Follow all the instructions found there to get the examples building and installing.

Once you have the Hothouse Example effects building and installing, copy the `Flick/` directory form this repo into the `HothouseExamples/src/` directory. Then, go into that directory:

1. Run `make`
2. Put your Hothouse into DFU mode
3. Run `make program-dfu`

### License

This software is licensed under the GNU General Public License (GPL) for open-source use. The ReverbSploodge (reverbsploodge.h/cpp) module is licensed using LVGL-2.1.