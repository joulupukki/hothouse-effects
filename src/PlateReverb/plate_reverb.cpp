/*
 * PlateReverb for Hothouse DSP Platform
 *
 * Copyright (c) 2024 Boyd Timothy. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
CONTROL       DESCRIPTION         NOTES
KNOB 1	      Reverb Dry Amount	
KNOB 2	      Reverb Wet Amount
KNOB 3	      Pre Delay Time
KNOB 4	      Input & Tank High Cut Frequency
KNOB 5	      Tank Mod Speed
KNOB 6	      Tank Mod Depth
SWITCH 1	    Input Diffusion       UP      - High
                                    MIDDLE  - Med
                                    DOWN    - Off
SWITCH 2	    Tank Diffusion        UP      - High
                                    MIDDLE  - Med
                                    DOWN    - Off
SWITCH 3	    Tank Mod Shape
                                    UP      - High
                                    MIDDLE  - Med
                                    DOWN    - Low
FOOTSWITCH 1  Input Diffusion On/Off
FOOTSWITCH 2	Reverb On/Off
 */

#include "daisysp.h"
#include "hothouse.h"
#include "Dattorro.hpp"

using clevelandmusicco::Hothouse;
using daisy::AudioHandle;
using daisy::Led;
using daisy::Parameter;
using daisy::SaiHandle;
using daisy::System;
using daisysp::DelayLine;
using daisysp::fonepole;

Hothouse hw;

// #define MAX_DELAY static_cast<size_t>(48000 * 2.0f) // 4 second max delay
#define MAX_DELAY static_cast<size_t>(32000 * 2.0f) // 4 second max delay

Dattorro verb(32000, 16, 4.0);
bool plateDiffusionEnabled = true;
double platePreDelay = 0.;
double previousPreDelay = 0.;

float plateWet = 0.5;
float plateDry = 0.5;

double plateDecay = 0.877465;
double plateTimeScale = 1.007500;

double plateDiffusion = 0.75;
double plateTempDiffusion = 0.625;

double plateTankDiffusion = 0.75;
double plateTankModShape = 0.75;

double plateInputDampLow = 0.;
double plateInputDampHigh = 10000.;

double plateDampLow = 0.0;
double plateDampHigh = 10000.;

double platePreviousInputDampLow = 0.;
double platePreviousInputDampHigh = 0.;

double platePreviousReverbDampLow = 0.;
double platePreviousReverbDampHigh = 0.;

double plateTankModSpeed = 1.0;
double plateTankModDepth = 1.0;

const double minus18dBGain = 0.12589254;
const double minus20dBGain = 0.1;

double leftInput = 0.;
double rightInput = 0.;
double leftOutput = 0.;
double rightOutput = 0.;

double inputAmplification = 1.0;

Parameter p_verb_dry, 
  p_verb_wet, 
  p_verb_pre_delay, 
  p_verb_high_cut_freq, 
  p_verb_mod_speed, 
  p_verb_mod_depth;

// Bypass vars
Led led_diffusion, led_verb;
bool bypass_diffusion = true;
bool bypass_verb = true;

inline float hardLimit100_(const float &x) {
    return (x > 1.) ? 1. : ((x < -1.) ? -1. : x);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  hw.ProcessAllControls();

  //
  // Set the state of the LEDs
  //
  if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
    bypass_diffusion = !bypass_diffusion;
    led_diffusion.Set(bypass_diffusion ? 0.0f : 1.0f);
  }
  led_diffusion.Update();

  if (hw.switches[Hothouse::FOOTSWITCH_2].RisingEdge()) {
    bypass_verb = !bypass_verb;
    led_verb.Set(bypass_verb ? 0.0f : 1.0f);
  }
  led_verb.Update();

  //
  // Read in all of the potentiometer values
  //
  plateDry = p_verb_dry.Process();
  plateWet = p_verb_wet.Process();
  platePreDelay = (double)p_verb_pre_delay.Process();
  plateInputDampHigh = plateDampHigh = (double)p_verb_high_cut_freq.Process() * 20000.0f;
  plateTankModSpeed = (double)p_verb_mod_speed.Process();
  plateTankModDepth = (double)p_verb_mod_depth.Process();

  //
  // Read in all of the toggle switch values
  //

  // Switch 1 - Input Diffusion
  static const double input_diffusion_values[] = {0.75f, 0.625f, 0.0f};
  plateDiffusion = input_diffusion_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)];
  plateDiffusionEnabled = plateDiffusion > 0.0;

  // Switch 2 - Tank Diffusion
  static const double tank_diffusion_values[] = {0.75f, 0.625f, 0.0f};
  plateTankDiffusion = tank_diffusion_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2)];

  // Switch 3 - Tank Mod Shape
  static const double tank_mod_shape_values[] = {0.75f, 0.5f, 0.25f};
  plateTankModShape = tank_mod_shape_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3)];

  verb.setPreDelay(platePreDelay);
  verb.setInputFilterHighCutoffPitch(plateInputDampHigh);
  verb.setTankFilterHighCutFrequency(plateDampHigh);
  verb.setTankModSpeed(plateTankModSpeed);
  verb.setTankModDepth(plateTankModDepth);
  verb.enableInputDiffusion(plateDiffusionEnabled);
  verb.setTankDiffusion(plateTankDiffusion);
  verb.setTankModShape(plateTankModShape);

  for (size_t i = 0; i < size; ++i) {
    // Dattorro seems to want to have values between -10 and 10 so times by 10
    leftInput = hardLimit100_(in[0][i]) * 10.;
    rightInput = hardLimit100_(in[1][i]) * 10.;

    verb.process(leftInput * minus18dBGain * minus20dBGain * (1.0 + inputAmplification * 7.) * clearPopCancelValue,
                  rightInput * minus18dBGain * minus20dBGain * (1.0 + inputAmplification * 7.) * clearPopCancelValue);

    leftOutput = ((leftInput * plateDry * 0.1) + (verb.getLeftOutput() * plateWet * clearPopCancelValue));
    rightOutput = ((rightInput * plateDry * 0.1) + (verb.getRightOutput() * plateWet * clearPopCancelValue));

    // gainControl(leftOutput, rightOutput);

    out[0][i] = leftOutput;
    out[1][i] = rightOutput;
  }
}

int main() {
  hw.Init();
  // hw.SetAudioBlockSize(48);  // Number of samples handled per callback
  // hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  hw.SetAudioBlockSize(32);  // Number of samples handled per callback
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);

  // Initialize LEDs
  led_diffusion.Init(hw.seed.GetPin(Hothouse::LED_1), false);
  led_verb.Init(hw.seed.GetPin(Hothouse::LED_2), false);

  // Initialize Potentiometers
  p_verb_dry.Init(hw.knobs[Hothouse::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
  p_verb_wet.Init(hw.knobs[Hothouse::KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
  p_verb_pre_delay.Init(hw.knobs[Hothouse::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
  p_verb_high_cut_freq.Init(hw.knobs[Hothouse::KNOB_4], 0.25f, 1.0f, Parameter::LINEAR);
  p_verb_mod_speed.Init(hw.knobs[Hothouse::KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
  p_verb_mod_depth.Init(hw.knobs[Hothouse::KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);

  // Plate Reverb Defaults
  verb.setSampleRate(32000);
  verb.setTimeScale(plateTimeScale);
  verb.setPreDelay(platePreDelay);
  verb.setInputFilterLowCutoffPitch(0.0);
  verb.setInputFilterHighCutoffPitch(10000.0);
  verb.enableInputDiffusion(plateDiffusionEnabled);
  verb.setDecay(plateDecay);
  verb.setTankDiffusion(plateDiffusion * 0.7);
  verb.setTankFilterLowCutFrequency(0);
  verb.setTankFilterHighCutFrequency(10000);
  verb.setTankModSpeed(1.0);
  verb.setTankModDepth(0.8);
  verb.setTankModShape(0.5);

  hw.StartAdc();
  hw.StartAudio(AudioCallback);

  while (true) {
    hw.DelayMs(10);

    // Toggle effect bypass LED when footswitch is pressed
    // led_bypass.Set(bypass ? 0.0f : 1.0f);
    // led_bypass.Update();

    // Call System::ResetToBootloader() if FOOTSWITCH_1 is pressed for 2 seconds
    hw.CheckResetToBootloader();
 }
  return 0;
}