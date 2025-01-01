/*
 * Flick for Hothouse DSP Platform
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
KNOB 1	      Reverb Dry/Wet Amount	
KNOB 2	      Tremolo Speed	
KNOB 3	      Tremolo Depth	
KNOB 4	      Delay Time	
KNOB 5	      Delay Feedback	
KNOB 6	      Delay Dry/Wet Amount	
SWITCH 1	    Reverb Feedback       UP - High           (100%)
                                    MIDDLE - Med        (90%)
                                    DOWN - Low          (50%)
SWITCH 2	    Reverb Tone           UP - Bright         (90%)
                                    MIDDLE - Neutral    (75%)
                                    DOWN - Dark         (60%)
SWITCH 3	    Sploodge
                                    UP - Shimmer        (90%)
                                    MIDDLE - Subtle     (25%)
                                    DOWN - Off          (0%)
FOOTSWITCH 1  Reverb On/Off	
FOOTSWITCH 2	Delay/Tremolo On/Off  Normal press toggles delay.
                                    Double press toggles tremolo.

                                    LED:
                                    - 100% when only relay is active
                                    - 40% pulsing when only tremolo is active
                                    - 100% pulsing when both are active
 */

#include "daisysp-lgpl.h"
#include "daisysp.h"
#include "hothouse.h"
#include "reverbsploodge.h"

using clevelandmusicco::Hothouse;
using daisy::AudioHandle;
using daisy::Led;
using daisy::Parameter;
using daisy::SaiHandle;
using daisy::System;
using daisysp::DelayLine;
using daisysp::fonepole;
using daisysp::Oscillator;
using daisysp::ReverbSploodge;
using daisysp::Tremolo;

Hothouse hw;

#define MAX_DELAY static_cast<size_t>(48000 * 2.0f) // 4 second max delay

ReverbSploodge verb;
Tremolo trem;
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMemL;
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMemR;
Parameter p_verb_amt;
Parameter p_trem_speed, p_trem_depth;
Parameter p_delay_time, p_delay_feedback, p_delay_amt;

struct Delay {
  DelayLine<float, MAX_DELAY> *del;
  float currentDelay;
  float delayTarget;
  float feedback;

  float Process(float in) {
    // set delay times
    fonepole(currentDelay, delayTarget, 0.0002f);
    del->SetDelay(currentDelay);

    float read = del->Read();
    del->Write((feedback * read) + in);

    return read;
  }
};

Delay delayL;
Delay delayR;
int delay_drywet;

float reverb_tone;
float reverb_feedback;
float reverb_sploodge;

// Bypass vars
Led led_verb, led_trem;
bool bypass_verb = true;
bool bypass_trem = true;
bool bypass_delay = true;

static const uint32_t LONG_PRESS_THRESHOLD = 2000; // 2 second hold time
static const uint32_t DOUBLE_PRESS_THRESHOLD = 700;  // milliseconds
bool footswitch2_last_state = false; // Assume initial state is open
int footswitch2_press_count = 0;
uint32_t footswitch2_start_time = 0;
uint32_t footswitch2_last_press_time = 0;
bool footswitch2_long_press_triggered = false;

void handle_normal_press() {
  bypass_delay = !bypass_delay;
}

void handle_double_press() {
  // When double press is detected, a normal press was already detected and
  // processed, so reverse that right off the bat.
  handle_normal_press();

  // Toggle the delay bypass
  bypass_trem = !bypass_trem;
}

void handle_long_press() {

}

/// @brief Watch Footswitch 2 for singla and double presses.
///
/// Toggle the tremolo on a single press and toggle the delay with a
/// double press.
void check_footswitch2_state(bool is_pressed) {
  // Detect press, double-press, and long press.

  uint32_t now = System::GetNow();

  if (is_pressed == true && footswitch2_last_state == false) {
    // Button pressed
    footswitch2_start_time = now;

    if ((now - footswitch2_last_press_time) <= DOUBLE_PRESS_THRESHOLD) {
      footswitch2_press_count++;
    } else {
      footswitch2_press_count = 1;
    }

    footswitch2_last_press_time = now;
    footswitch2_long_press_triggered = false; // Reset long press trigger when pressed
  }

  uint32_t press_duration = now - footswitch2_start_time;

  if (is_pressed == true && press_duration >= LONG_PRESS_THRESHOLD && !footswitch2_long_press_triggered) {
    // Footswitch is being held down
    handle_long_press();
    footswitch2_long_press_triggered = true; // Ensure long press is only triggered once
  }

  if (is_pressed == false && footswitch2_last_state == true) {
    // Button released
    if (!footswitch2_long_press_triggered) {
      if (footswitch2_press_count >= 2) {
        handle_double_press();
        footswitch2_press_count = 0;
      } else if (press_duration < LONG_PRESS_THRESHOLD) {
        handle_normal_press();
      }
    }
  }

  footswitch2_last_state = is_pressed;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  static float trem_val;
  float verb_amt;
  hw.ProcessAllControls();

  if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
    bypass_verb = !bypass_verb;
    led_verb.Set(bypass_verb ? 0.0f : 1.0f);
  }
  led_verb.Update();

  check_footswitch2_state(hw.switches[Hothouse::FOOTSWITCH_2].RisingEdge());

  // Reduce number of LED Updates for pulsing trem LED
  {
    static int count = 0;
    // set led 100 times/sec
    if (++count == hw.AudioCallbackRate() / 100) {
      count = 0;
      // If just delay is on, show full-strength LED
      // If just trem is on, show 40% pulsing LED
      // If both are on, show 100% pulsing LED
      led_trem.Set(bypass_trem ? bypass_delay ? 0.0f : 1.0 : bypass_delay ? trem_val * 0.4 : trem_val);
    }
  }
  led_trem.Update();

  static const float reverb_feedback_values[] = {1.0f, 0.9f, 0.5f};
  reverb_feedback = reverb_feedback_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)];
  verb.SetFeedback(reverb_feedback);

  static const float reverb_tone_values[] = {0.9f, 0.75f, 0.6f, };
  reverb_tone = reverb_tone_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2)];
  verb.SetWetTone(reverb_tone);

  static const float reverb_sploodge_values[] = {0.9f, 0.25f, 0.0f, };
  reverb_sploodge = reverb_sploodge_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3)];
  verb.SetSploodge(reverb_sploodge);

  trem.SetFreq(p_trem_speed.Process());
  trem.SetDepth(p_trem_depth.Process());

  //
  // Delay
  //
  delayL.delayTarget = delayR.delayTarget =  p_delay_time.Process();
  delayL.feedback = delayR.feedback = p_delay_feedback.Process();
  delay_drywet = (int)p_delay_amt.Process();

  for (size_t i = 0; i < size; ++i) {
    float dry_L = in[0][i];
    float dry_R = in[1][i];
    float s_L, s_R;
    s_L = dry_L;
    s_R = dry_R;

    if (!bypass_delay) {
      float mixL = 0;
      float mixR = 0;
      float fdrywet = (float)delay_drywet / 100.0f;

      // update delayline with feedback
      float sigL = delayL.Process(s_L);
      float sigR = delayR.Process(s_R);
      mixL += sigL;
      mixR += sigR;

      // apply drywet and attenuate
      s_L = fdrywet * mixL * 0.333f + (1.0f - fdrywet) * s_L;
      s_R = fdrywet * mixR * 0.333f + (1.0f - fdrywet) * s_R;
      // s = fdrywet * mix * 0.333f + (1.0f - fdrywet) * s;
    }

    if (!bypass_trem) {
      // trem_val gets used above for pulsing LED
      trem_val = trem.Process(1.f);
      s_L = s_L * trem_val;
      s_R = s_R * trem_val;
      // s = s * trem_val;
    }
    if (!bypass_verb) {
      float out_L, out_R;
      verb.Process(s_L, s_R, &out_L, &out_R);
      verb_amt = p_verb_amt.Process();
      s_L = (s_L * (1.0f - verb_amt) + verb_amt * out_L);
      s_R = (s_R * (1.0f - verb_amt) + verb_amt * out_R);

      // s_L = (s_L * plateDry) + (out_L * plateWet * clearPopCancelValue);
      // s_R = (s_R * plateDry) + (out_R * plateWet * clearPopCancelValue);
    }

    out[0][i] = s_L;
    out[1][i] = s_R;
    // Quick and dirty dual-mono
    // out[0][i] = out[1][i] = s;
  }

  // for (size_t i = 0; i < size; ++i) {
  //   if (bypass) {
  //     // Copy left input to both outputs (mono-to-dual-mono)
  //     out[0][i] = out[1][i] = in[0][i];
  //   } else {
  //     // TODO: replace silence with something awesome
  //     out[0][i] = out[1][i] = 0.0f;
  //   }
  // }
}

int main() {
  hw.Init(true); // Init the CPU at full speed
  hw.SetAudioBlockSize(48);  // Number of samples handled per callback
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  
  // Initialize LEDs
  led_verb.Init(hw.seed.GetPin(Hothouse::LED_1), false);
  led_trem.Init(hw.seed.GetPin(Hothouse::LED_2), false);

  // Initialize Potentiometers
  p_verb_amt.Init(hw.knobs[Hothouse::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);

  p_trem_speed.Init(hw.knobs[Hothouse::KNOB_2], 0.2f, 16.0f, Parameter::LINEAR);
  p_trem_depth.Init(hw.knobs[Hothouse::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);

  p_delay_time.Init(hw.knobs[Hothouse::KNOB_4], hw.AudioSampleRate() * 0.05f, MAX_DELAY, Parameter::LOGARITHMIC);
  p_delay_feedback.Init(hw.knobs[Hothouse::KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
  p_delay_amt.Init(hw.knobs[Hothouse::KNOB_6], 0.0f, 100.0f, Parameter::LINEAR);

  trem.Init(hw.AudioSampleRate());
  trem.SetWaveform(Oscillator::WAVE_SIN); // Only sine wave supported

  verb.Init(hw.AudioSampleRate());
  verb.SetFeedback(0.87);
  
  delMemL.Init();
  delMemR.Init();
  delayL.del = &delMemL;
  delayR.del = &delMemR;

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