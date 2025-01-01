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

#include "daisysp.h"
#include "hothouse.h"
#include "Dattorro.hpp"

using clevelandmusicco::Hothouse;
using daisy::AudioHandle;
using daisy::Led;
using daisy::Parameter;
using daisy::SaiHandle;
using daisy::System;

Hothouse hw;

// Dattorro verb(32000, 16, 4.0);
Dattorro verb(48000, 16, 4.0);

bool plateDiffusionEnabled = true;
double platePreDelay = 0.;

double plateDelay = 0.0;

float plateDry = 1.0;
float plateWet = 0.5;

double plateDecay = 0.67;
double plateTimeScale = 1.007500;

double plateTankDiffusion = 0.7;

  /**
   * Good Defaults
   * Lo Pitch: .287 (2.87) = 100Hz: 440 * (2^(2.87-5))
   * InputFilterHighCutoffPitch: 0.77 (7.77) is approx 3000Hz
   * TankFilterHighCutFrequency: 0.8 (8.0) is 3520Hz
   * 0.9507 is approx 10kHz
   * 
   * mod speed: 0.5
   * mod depth: 0.5
   * mod shape: 0.75
   */

// The damping values appear to be want to be between 0 and 10
double plateInputDampLow = 2.87; // approx 100Hz
double plateInputDampHigh = 6.77; // approx 1.5kHz

double plateTankDampLow = 2.87; // approx 100Hz
double plateTankDampHigh = 6.77; // approx 1.5kHz

double plateTankModSpeed = 1.0;
double plateTankModDepth = 0.5;
double plateTankModShape = 0.75;

const double minus18dBGain = 0.12589254;
const double minus20dBGain = 0.1;

double leftInput = 0.;
double rightInput = 0.;
double leftOutput = 0.;
double rightOutput = 0.;

double inputAmplification = 1.0;

Parameter p_knob_1, p_knob_2, p_knob_3, p_knob_4, p_knob_5, p_knob_6;

// Parameter p_verb_dry, 
//   p_verb_wet, 
//   p_verb_delay, 
//   p_verb_high_cut_freq, 
//   p_verb_mod_speed, 
//   p_verb_mod_depth;

// Bypass vars
Led led_100p_wet, led_verb;
bool bypass_100p_wet = true;
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
    bypass_100p_wet = !bypass_100p_wet;
    led_100p_wet.Set(bypass_100p_wet ? 0.0f : 1.0f);
  }
  led_100p_wet.Update();

  if (hw.switches[Hothouse::FOOTSWITCH_2].RisingEdge()) {
    bypass_verb = !bypass_verb;
    led_verb.Set(bypass_verb ? 0.0f : 1.0f);
  }
  led_verb.Update();

  //
  // Read in all of the potentiometer values
  //
  if (bypass_100p_wet) {
    plateDry = p_knob_1.Process();
    plateWet = p_knob_2.Process();    
  } else {
    plateDry = 0.0;
    plateWet = 1.0;
  }
  plateDecay = p_knob_3.Process();
  plateTankDiffusion = p_knob_4.Process();
  plateInputDampHigh = p_knob_5.Process() * 10.0; // Dattorro takes values for this between 0 and 10
  plateTankDampHigh = p_knob_6.Process() * 10.0; // Dattorro takes values for this between 0 and 10

  //
  // Read in all of the toggle switch values
  //

  // Switch 1 - Tank Mod Speed
  static const double tank_mod_speed_values[] = {1.0f, 0.5f, 0.0f};
  plateTankModSpeed = tank_mod_speed_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)];

  // Switch 2 - Tank Mod Depth
  static const double tank_mod_depth_values[] = {1.0f, 0.5f, 0.0f};
  plateTankModDepth = tank_mod_depth_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2)];

  // Switch 3 - Pre Delay
  static const double pre_delay_values[] = {0.1f, 0.05f, 0.0f};
  platePreDelay = pre_delay_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3)];

  if (!bypass_verb) {
    verb.setDecay(plateDecay);
    verb.setTankDiffusion(plateTankDiffusion);
    verb.setInputFilterHighCutoffPitch(plateInputDampHigh);
    verb.setTankFilterHighCutFrequency(plateTankDampHigh);

    verb.setTankModSpeed(plateTankModSpeed);
    verb.setTankModDepth(plateTankModDepth);
    verb.setPreDelay(platePreDelay);    

    for (size_t i = 0; i < size; ++i) {
      // Dattorro seems to want to have values between -10 and 10 so times by 10
      leftInput = hardLimit100_(in[0][i]) * 10.;
      rightInput = hardLimit100_(in[1][i]) * 10.;

      verb.process(leftInput * minus18dBGain * minus20dBGain * (1.0 + inputAmplification * 7.) * clearPopCancelValue,
                    rightInput * minus18dBGain * minus20dBGain * (1.0 + inputAmplification * 7.) * clearPopCancelValue);

      leftOutput = ((leftInput * plateDry * 0.1) + (verb.getLeftOutput() * plateWet * clearPopCancelValue));
      rightOutput = ((rightInput * plateDry * 0.1) + (verb.getRightOutput() * plateWet * clearPopCancelValue));

      out[0][i] = (float)leftOutput;
      out[1][i] = (float)rightOutput;
    }
  } else {
    for (size_t i = 0; i < size; ++i) {
      out[0][i] = in[0][i];
      out[1][i] = in[1][i];
    }
  }
}

int main() {
  hw.Init(true);
  hw.SetAudioBlockSize(48);  // Number of samples handled per callback
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  // hw.SetAudioBlockSize(32);  // Number of samples handled per callback
  // hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_32KHZ);

  // Initialize LEDs
  led_100p_wet.Init(hw.seed.GetPin(Hothouse::LED_1), false);
  led_verb.Init(hw.seed.GetPin(Hothouse::LED_2), false);

  p_knob_1.Init(hw.knobs[Hothouse::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_2.Init(hw.knobs[Hothouse::KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_3.Init(hw.knobs[Hothouse::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_4.Init(hw.knobs[Hothouse::KNOB_4], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_5.Init(hw.knobs[Hothouse::KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_6.Init(hw.knobs[Hothouse::KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);

  // Zero out the InterpDelay buffers used by the plate reverb
  for(int i = 0; i < 50; i++) {
      for(int j = 0; j < 144000; j++) {
          sdramData[i][j] = 0.;
      }
  }

  // Set this to 1.0 or plate reverb won't work. This is defined in Dattorro's
  // InterpDelay.cpp file.
  hold = 1.;

  // Plate Reverb Defaults
  verb.setSampleRate(48000);
  // verb.setSampleRate(32000);
  verb.setTimeScale(plateTimeScale);
  verb.setPreDelay(platePreDelay);
  verb.setInputFilterLowCutoffPitch(0.0);
  verb.setInputFilterHighCutoffPitch(10000.0);
  verb.enableInputDiffusion(plateDiffusionEnabled);
  verb.setDecay(plateDecay);
  verb.setTankDiffusion(plateTankDiffusion);
  verb.setTankFilterLowCutFrequency(0);
  verb.setTankFilterHighCutFrequency(10000);
  verb.setTankModSpeed(plateTankModSpeed);
  verb.setTankModDepth(plateTankModDepth);
  verb.setTankModShape(plateTankModShape);

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