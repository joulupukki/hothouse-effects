/*
 * Mutable Rings for Hothouse DSP Platform
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
#include "ap_demo.hpp"
#include "common.hpp"
#include "datorro_plate.hpp"
#include "mutable_rings.hpp"

#include <algorithm>

using clevelandmusicco::Hothouse;
using daisy::AudioHandle;
using daisy::Led;
using daisy::Parameter;
using daisy::SaiHandle;
using daisy::System;

Hothouse hw;

std::array<float, 32768> DSY_SDRAM_BSS delay_line_buffer;
MutableRings reverb_(delay_line_buffer);
DatorroPlate plate_(delay_line_buffer);
AllPassDemo apdemo_(delay_line_buffer);

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

// void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
//                   size_t size) {
void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out,
                   size_t blocksize) {
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

  const float strength = p_knob_1.Process();
  const float size = p_knob_2.Process();
  const float shape = p_knob_3.Process();

  std::copy_n(in, blocksize, out);

  if (!bypass_verb) {
    // std::fill_n(out, blocksize, 0.0f);
    StereoSignal in_stereo{reinterpret_cast<const StereoSample*>(in), blocksize / 2};
    StereoBuffer out_stereo{reinterpret_cast<StereoSample*>(out), blocksize / 2};

    static const int effect_type_values[] = {0, 1, 2};
    int effect_type = effect_type_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)];

    switch(effect_type) {
      case 0:
        reverb_.set_amount(strength * 0.5f);
        reverb_.set_time(0.35f + 0.63f * size);
        reverb_.set_input_gain(0.2f);
        reverb_.set_lp(0.3f + shape * 0.6f);
        reverb_.Process(in_stereo, out_stereo);
        break;
      case 1:
        plate_.set_amount(strength * 0.5f);
        plate_.set_time(0.35f + 0.65f * size);
        plate_.set_input_gain(0.2f);
        plate_.set_lp(0.3f + shape * 0.7f);
        plate_.Process(in_stereo, out_stereo);
        break;
      case 2:
        apdemo_.set_amount(strength);
        apdemo_.set_input_gain(0.2f);
        apdemo_.set_size(size);
        apdemo_.set_diffusion(shape);
        apdemo_.Process(in_stereo, out_stereo);
        break;
    }
  }
}

int main() {
  hw.Init(true);
  hw.SetAudioBlockSize(48);  // Number of samples handled per callback
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

  // Initialize LEDs
  led_100p_wet.Init(hw.seed.GetPin(Hothouse::LED_1), false);
  led_verb.Init(hw.seed.GetPin(Hothouse::LED_2), false);

  p_knob_1.Init(hw.knobs[Hothouse::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_2.Init(hw.knobs[Hothouse::KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_3.Init(hw.knobs[Hothouse::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_4.Init(hw.knobs[Hothouse::KNOB_4], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_5.Init(hw.knobs[Hothouse::KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_6.Init(hw.knobs[Hothouse::KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);

  delay_line_buffer.fill(0);

  reverb_.Init(hw.AudioSampleRate());
  plate_.Init(hw.AudioSampleRate());
  apdemo_.Init(hw.AudioSampleRate());
 
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