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

#include "daisy.h"
#include "daisysp.h"
#include "hothouse.h"
#include "Dattorro.hpp"

using clevelandmusicco::Hothouse;
using daisy::AudioHandle;
using daisy::Led;
using daisy::Parameter;
using daisy::PersistentStorage;
using daisy::SaiHandle;
using daisy::System;
using daisysp::DelayLine;
using daisysp::fonepole;
using daisysp::Oscillator;
using daisysp::Tremolo;

/// Increment this when changing the settings struct so the software will know
/// to reset to defaults if this ever changes.
#define SETTINGS_VERSION 1

Hothouse hw;

#define MAX_DELAY static_cast<size_t>(48000 * 2.0f) // 4 second max delay

enum ReverbMode {
  REVERB_MODE_NORMAL, // Normal reverb mode
  REVERB_MODE_EDIT,   // Edit mode activated by float-press of the left foot switch
};

// Persistent Settings
struct Settings {
  int version; // Version of the settings struct
  float decay;
  float diffusion;
  float inputCutoffFreq;
  float tankCutoffFreq;
  float tankModSpeed;
  float tankModDepth;
  float tankModShape;
  float preDelay;

	//Overloading the != operator
	//This is necessary as this operator is used in the PersistentStorage source code
	bool operator!=(const Settings& a) const {
    return !(
      a.version == version &&
      a.decay == decay &&
      a.diffusion == diffusion &&
      a.inputCutoffFreq == inputCutoffFreq &&
      a.tankCutoffFreq == tankCutoffFreq &&
      a.tankModSpeed == tankModSpeed &&
      a.tankModDepth == tankModDepth &&
      a.tankModShape == tankModShape &&
      a.preDelay == preDelay
    );
  }
};

//Persistent Storage Declaration. Using type Settings and passed the devices qspi handle
PersistentStorage<Settings> SavedSettings(hw.seed.qspi);

Tremolo trem;
uint8_t trem_waveform = Oscillator::WAVE_SIN;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMemL;
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMemR;

Dattorro verb(48000, 16, 4.0);
ReverbMode verb_mode = REVERB_MODE_NORMAL;

Parameter p_verb_amt;
Parameter p_trem_speed, p_trem_depth;
Parameter p_delay_time, p_delay_feedback, p_delay_amt;

Parameter p_knob_1, p_knob_2, p_knob_3, p_knob_4, p_knob_5, p_knob_6;

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
Led led_left, led_right;
bool bypass_verb = true;
bool bypass_trem = true;
bool bypass_delay = true;

// Reverb vars
bool plateDiffusionEnabled = true;
float platePreDelay = 0.;

float plateDelay = 0.0;

// float plateDry = 1.0;
float plateWet = 0.5;

float plateDecay = 0.67;
float plateTimeScale = 1.007500;

float plateTankDiffusion = 0.7;

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
float plateInputDampLow = 2.87; // approx 100Hz
float plateInputDampHigh = 6.77; // approx 1.5kHz

float plateTankDampLow = 2.87; // approx 100Hz
float plateTankDampHigh = 6.77; // approx 1.5kHz

float plateTankModSpeed = 1.0;
float plateTankModDepth = 0.5;
float plateTankModShape = 0.75;

const float minus18dBGain = 0.12589254;
const float minus20dBGain = 0.1;

float leftInput = 0.;
float rightInput = 0.;
float leftOutput = 0.;
float rightOutput = 0.;

float inputAmplification = 1.0; // This isn't really used yet

static const uint32_t LONG_PRESS_THRESHOLD = 2000; // 2 second hold time
static const uint32_t DOUBLE_PRESS_THRESHOLD = 600;  // milliseconds
bool footswitch_last_state[] = {false, false}; // Assume initial state is open
int footswitch_press_count[] = {0, 0};
uint32_t footswitch_start_time[] = {0, 0};
uint32_t footswitch_last_press_time[] = {0, 0};
bool footswitch_long_press_triggered[] = {false, false};

bool trigger_settings_save = false;

/// @brief Used at startup to control a factory reset.
///
/// This gets set to true in `main()` if footswitch 2 is depressed at boot.
/// The LED lights will start flashing alternatively. To exit this mode without
/// making any changes, press either footswitch.
///
/// To reset, rotate knob_1 to 100%, to 0%, to 100%, and back to 0%. This will
/// restore all defaults and then go into normal pedal mode.
bool is_factory_reset_mode = false;

/// @brief Tracks the stage of knob_1 rotation in factory reset mode.
///
/// 0: User must rotate knob_1 to 100% to advance to the next stage.
/// 1: User must rotate knob_1 to 0% to advance to the next stage.
/// 2: User must rotate knob_1 to 100% to advance to the next stage.
/// 3: User must rotate knob_1 to 0% to complete the factory reset.
int factory_reset_stage = 0;

void load_settings() {

	// Reference to local copy of settings stored in flash
	Settings &LocalSettings = SavedSettings.GetSettings();

  // int version; // Version of the settings struct
  // float decay;
  // float diffusion;
  // float inputCutoffFreq;
  // float tankCutoffFreq;
  // float tankModSpeed;
  // float tankModDepth;
  // float tankModShape;
  // float preDelay;

  int savedVersion = LocalSettings.version;

  if (savedVersion != SETTINGS_VERSION) {
    // Something has changed. Load defaults!
    SavedSettings.RestoreDefaults();
    load_settings();
    return;
  }

  plateDecay = LocalSettings.decay;
  plateTankDiffusion = LocalSettings.diffusion;
  plateInputDampHigh = LocalSettings.inputCutoffFreq;
  plateTankDampHigh = LocalSettings.tankCutoffFreq;
  plateTankModSpeed = LocalSettings.tankModSpeed;
  plateTankModDepth = LocalSettings.tankModDepth;
  plateTankModShape = LocalSettings.tankModShape;
  platePreDelay = LocalSettings.preDelay;

  verb.setPreDelay(platePreDelay);
  verb.setInputFilterHighCutoffPitch(plateInputDampHigh);
  verb.setDecay(plateDecay);
  verb.setTankDiffusion(plateTankDiffusion);
  verb.setTankFilterHighCutFrequency(plateTankDampHigh);
  verb.setTankModSpeed(plateTankModSpeed);
  verb.setTankModDepth(plateTankModDepth);
  verb.setTankModShape(plateTankModShape);
}

void save_settings() {
	//Reference to local copy of settings stored in flash
	Settings &LocalSettings = SavedSettings.GetSettings();

  LocalSettings.version = SETTINGS_VERSION;
  LocalSettings.decay = plateDecay;
  LocalSettings.diffusion = plateTankDiffusion;
  LocalSettings.inputCutoffFreq = plateInputDampHigh;
  LocalSettings.tankCutoffFreq = plateTankDampHigh;
  LocalSettings.tankModSpeed = plateTankModSpeed;
  LocalSettings.tankModDepth = plateTankModDepth;
  LocalSettings.tankModShape = plateTankModShape;
  LocalSettings.preDelay = platePreDelay;

	trigger_settings_save = true;
}

void handle_normal_press(Hothouse::Switches footswitch) {
  if (verb_mode == REVERB_MODE_EDIT) {
    // If either of the footswitches are pressed, save the reverb settings and
    // exit from reverb edit mode.
    save_settings();
    verb_mode = REVERB_MODE_NORMAL;
  } else {
    if (footswitch == Hothouse::FOOTSWITCH_1) {
      bypass_verb = !bypass_verb;
    } else {
      bypass_delay = !bypass_delay;
    }
  }
}

void handle_float_press(Hothouse::Switches footswitch) {
  // Ignore float presses in reverb edit mode
  if (verb_mode == REVERB_MODE_EDIT) {
    return;
  }

  // When float press is detected, a normal press was already detected and
  // processed, so reverse that right off the bat.
  handle_normal_press(footswitch);

  if (footswitch == Hothouse::FOOTSWITCH_1) {
    // Go into reverb edit mode
    bypass_verb = false; // Make sure that reverb is ON
    verb_mode = REVERB_MODE_EDIT;
  } else if (footswitch == Hothouse::FOOTSWITCH_2) {
    // Toggle the trem bypass
    bypass_trem = !bypass_trem;
  }
}

void handle_long_press(Hothouse::Switches footswitch) {
  // Intentionally blank
}

/// @brief Watch Footswitches for singla and float presses.
void check_footswitch_state(Hothouse::Switches footswitch, bool is_pressed) {  
  int footswitch_index = footswitch == Hothouse::FOOTSWITCH_1 ? 0 : 1;

  uint32_t now = System::GetNow();

  if (is_pressed == true && footswitch_last_state[footswitch_index] == false) {
    // Button pressed
    footswitch_start_time[footswitch_index] = now;

    if ((now - footswitch_last_press_time[footswitch_index]) <= DOUBLE_PRESS_THRESHOLD) {
      footswitch_press_count[footswitch_index]++;
    } else {
      footswitch_press_count[footswitch_index] = 1;
    }

    footswitch_last_press_time[footswitch_index] = now;
    footswitch_long_press_triggered[footswitch_index] = false; // Reset long press trigger when pressed
  }

  uint32_t press_duration = now - footswitch_start_time[footswitch_index];

  if (is_pressed == true && press_duration >= LONG_PRESS_THRESHOLD && !footswitch_long_press_triggered[footswitch_index]) {
    // Footswitch is being held down
    handle_long_press(footswitch);
    footswitch_long_press_triggered[footswitch_index] = true; // Ensure long press is only triggered once
  }

  if (is_pressed == false && footswitch_last_state[footswitch_index] == true) {
    // Button released
    if (!footswitch_long_press_triggered[footswitch_index]) {
      if (footswitch_press_count[footswitch_index] >= 2) {
        handle_float_press(footswitch);
        footswitch_press_count[footswitch_index] = 0;
      } else if (press_duration < LONG_PRESS_THRESHOLD) {
        handle_normal_press(footswitch);
      }
    }
  }

  footswitch_last_state[footswitch_index] = is_pressed;
}

inline float hardLimit100_(const float &x) {
    return (x > 1.) ? 1. : ((x < -1.) ? -1. : x);
}

void quick_led_flash() {
  led_left.Set(1.0f);
  led_right.Set(1.0f);
  led_left.Update();
  led_right.Update();
  hw.DelayMs(500);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  static float trem_val;
  // float verb_amt;
  hw.ProcessAllControls();

  if (verb_mode == REVERB_MODE_EDIT) {
    // Edit mode

    // Blink the left & right LEDs
    {
      static uint32_t edit_count = 0;
      static bool led_state = true;
      if (++edit_count >= hw.AudioCallbackRate() / 2) {
        edit_count = 0;
        led_state = !led_state;
        led_left.Set(led_state ? 1.0f : 0.0f);
        led_right.Set(led_state ? 1.0f : 0.0f);
      }
    }
  } else {
    // Normal mode
    led_left.Set(bypass_verb ? 0.0f : 1.0f);

    // Reduce number of LED Updates for pulsing trem LED
    {
      static int count = 0;
      // set led 100 times/sec
      if (++count == hw.AudioCallbackRate() / 100) {
        count = 0;
        // If just delay is on, show full-strength LED
        // If just trem is on, show 40% pulsing LED
        // If both are on, show 100% pulsing LED
        led_right.Set(bypass_trem ? bypass_delay ? 0.0f : 1.0 : bypass_delay ? trem_val * 0.4 : trem_val);
      }
    }
  }
  led_left.Update();
  led_right.Update();

  // Handles all footswitch presses and float-presses
  check_footswitch_state(Hothouse::FOOTSWITCH_1, hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge());
  check_footswitch_state(Hothouse::FOOTSWITCH_2, hw.switches[Hothouse::FOOTSWITCH_2].RisingEdge());

  if (verb_mode == REVERB_MODE_NORMAL) {
    trem.SetFreq(p_trem_speed.Process());
    trem.SetDepth(p_trem_depth.Process());

    // static const uint8_t trem_waveforms[] = {Oscillator::WAVE_SQUARE_ROUNDED, Oscillator::WAVE_TRI, Oscillator::WAVE_SIN};
    static const uint8_t trem_waveforms[] = {Oscillator::WAVE_SQUARE, Oscillator::WAVE_TRI, Oscillator::WAVE_SIN};
    trem_waveform = trem_waveforms[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2)];
    trem.SetWaveform(trem_waveform);

    //
    // Delay
    //
    delayL.delayTarget = delayR.delayTarget =  p_delay_time.Process();
    delayL.feedback = delayR.feedback = p_delay_feedback.Process();
    delay_drywet = (int)p_delay_amt.Process();
  }

  plateWet = p_verb_amt.Process();
  // plateDry = 1.0 - plateWet;
  // s_L = (s_L * (1.0f - verb_amt) + verb_amt * out_L);
  // s_R = (s_R * (1.0f - verb_amt) + verb_amt * out_R);

  if (verb_mode == REVERB_MODE_EDIT) {
    // Edit mode
    // plateDry = p_knob_1.Process();
    plateWet = p_knob_1.Process();
    platePreDelay = p_knob_2.Process() * 0.25;
    plateDecay = p_knob_3.Process();        
    plateTankDiffusion = p_knob_4.Process();
    plateInputDampHigh = p_knob_5.Process() * 10.0; // Dattorro takes values for this between 0 and 10
    plateTankDampHigh = p_knob_6.Process() * 10.0; // Dattorro takes values for this between 0 and 10

    //
    // Read in all of the toggle switch values
    //

    // Switch 1 - Tank Mod Speed
    static const float tank_mod_speed_values[] = {1.0f, 0.5f, 0.0f};
    plateTankModSpeed = tank_mod_speed_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)];

    // Switch 2 - Tank Mod Depth
    static const float tank_mod_depth_values[] = {1.0f, 0.5f, 0.0f};
    plateTankModDepth = tank_mod_depth_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2)];

    // Switch 3 - Tank Mod Shape
    static const float tank_mod_shape_values[] = {1.0f, 0.75f, 0.0f};
    plateTankModShape = tank_mod_shape_values[hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3)];

    verb.setDecay(plateDecay);
    verb.setTankDiffusion(plateTankDiffusion);
    verb.setInputFilterHighCutoffPitch(plateInputDampHigh);
    verb.setTankFilterHighCutFrequency(plateTankDampHigh);

    verb.setTankModSpeed(plateTankModSpeed);
    verb.setTankModDepth(plateTankModDepth);
    verb.setTankModShape(plateTankModShape);
    verb.setPreDelay(platePreDelay);    
  }

  for (size_t i = 0; i < size; ++i) {
    float dry_L = in[0][i];
    float dry_R = in[1][i];
    float s_L, s_R;
    s_L = dry_L;
    s_R = dry_R;

    if (!bypass_delay) {
      float mixL = 0;
      float mixR = 0;
      float fdrywet = delay_drywet / 100.0f;

      // update delayline with feedback
      float sigL = delayL.Process(s_L);
      float sigR = delayR.Process(s_R);
      mixL += sigL;
      mixR += sigR;

      float delay_make_up_gain = 1.66;

      // apply drywet and attenuate
      s_L = fdrywet * mixL * 0.333f + (1.0f - fdrywet) * s_L * delay_make_up_gain;
      s_R = fdrywet * mixR * 0.333f + (1.0f - fdrywet) * s_R * delay_make_up_gain;
    }

    if (!bypass_trem) {
      // trem_val gets used above for pulsing LED
      trem_val = trem.Process(1.f);
      float trem_make_up_gain = 1.2;
      s_L = s_L * trem_val * trem_make_up_gain;
      s_R = s_R * trem_val * trem_make_up_gain;
      // s = s * trem_val;
    }
    if (!bypass_verb) {
      // float out_L, out_R;
      // verb.Process(s_L, s_R, &out_L, &out_R);
      // verb_amt = p_verb_amt.Process();

      // Dattorro seems to want to have values between -10 and 10 so times by 10
      leftInput = hardLimit100_(s_L) * 10.0f;
      rightInput = hardLimit100_(s_R) * 10.0f;

      verb.process(leftInput * minus18dBGain * minus20dBGain * (1.0f + inputAmplification * 7.0f) * clearPopCancelValue,
                    rightInput * minus18dBGain * minus20dBGain * (1.0f + inputAmplification * 7.0f) * clearPopCancelValue);

      // leftOutput = ((leftInput * plateDry * 0.1) + (verb.getLeftOutput() * plateWet * clearPopCancelValue));
      // rightOutput = ((rightInput * plateDry * 0.1) + (verb.getRightOutput() * plateWet * clearPopCancelValue));
      leftOutput = ((leftInput * 0.1) + (verb.getLeftOutput() * plateWet * clearPopCancelValue));
      rightOutput = ((rightInput * 0.1) + (verb.getRightOutput() * plateWet * clearPopCancelValue));

      s_L = leftOutput;
      s_R = rightOutput;
    }

    out[0][i] = s_L;
    out[1][i] = s_R;
  }
}

int main() {
  hw.Init(true); // Init the CPU at full speed
  hw.SetAudioBlockSize(8);  // Number of samples handled per callback
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  
  // Initialize LEDs
  led_left.Init(hw.seed.GetPin(Hothouse::LED_1), false);
  led_right.Init(hw.seed.GetPin(Hothouse::LED_2), false);

  //
  // Initialize Potentiometers
  //

  // The p_knob_n parameters are used to process the potentiometers when in reverb edit mode.
  p_knob_1.Init(hw.knobs[Hothouse::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_2.Init(hw.knobs[Hothouse::KNOB_2], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_3.Init(hw.knobs[Hothouse::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_4.Init(hw.knobs[Hothouse::KNOB_4], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_5.Init(hw.knobs[Hothouse::KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
  p_knob_6.Init(hw.knobs[Hothouse::KNOB_6], 0.0f, 1.0f, Parameter::LINEAR);

  p_verb_amt.Init(hw.knobs[Hothouse::KNOB_1], 0.0f, 1.0f, Parameter::LINEAR);

  p_trem_speed.Init(hw.knobs[Hothouse::KNOB_2], 0.2f, 16.0f, Parameter::LINEAR);
  p_trem_depth.Init(hw.knobs[Hothouse::KNOB_3], 0.0f, 1.0f, Parameter::LINEAR);

  p_delay_time.Init(hw.knobs[Hothouse::KNOB_4], hw.AudioSampleRate() * 0.05f, MAX_DELAY, Parameter::LOGARITHMIC);
  p_delay_feedback.Init(hw.knobs[Hothouse::KNOB_5], 0.0f, 1.0f, Parameter::LINEAR);
  p_delay_amt.Init(hw.knobs[Hothouse::KNOB_6], 0.0f, 100.0f, Parameter::LINEAR);

  delMemL.Init();
  delMemR.Init();
  delayL.del = &delMemL;
  delayR.del = &delMemR;

  trem.Init(hw.AudioSampleRate());
  trem.SetWaveform(trem_waveform); // Only sine wave supported

  //
  // Dattorro Reverb Initialization
  //
  // Zero out the InterpDelay buffers used by the plate reverb
  for(int i = 0; i < 50; i++) {
      for(int j = 0; j < 144000; j++) {
          sdramData[i][j] = 0.;
      }
  }
  // Set this to 1.0 or plate reverb won't work. This is defined in Dattorro's
  // InterpDelay.cpp file.
  hold = 1.;

  verb.setSampleRate(48000);
  verb.setTimeScale(plateTimeScale);
  verb.enableInputDiffusion(plateDiffusionEnabled);
  verb.setInputFilterLowCutoffPitch(plateInputDampLow);
  verb.setTankFilterLowCutFrequency(plateTankDampLow);

  Settings defaultSettings = {
    SETTINGS_VERSION, // version
    plateDecay,
    plateTankDiffusion,
    plateInputDampHigh,
    plateTankDampHigh,
    plateTankModSpeed,
    plateTankModDepth,
    plateTankModShape,
    platePreDelay
  };
  SavedSettings.Init(defaultSettings);

  load_settings();

  hw.StartAdc();
  hw.ProcessAllControls();
  if (hw.switches[Hothouse::FOOTSWITCH_2].RawState()) {
    is_factory_reset_mode = true;
  } else {
    hw.StartAudio(AudioCallback);
  }
  
  while (true) {
    if(trigger_settings_save) {
			SavedSettings.Save(); // Writing locally stored settings to the external flash
			trigger_settings_save = false;
		} else if (is_factory_reset_mode) {
      hw.ProcessAllControls();

      // if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge() || hw.switches[Hothouse::FOOTSWITCH_2].RisingEdge()) {
      //   // Exit firmware reset mode
      //   hw.StartAudio(AudioCallback);
      //   factory_reset_stage = 0;
      //   is_factory_reset_mode = false;
      // } else {
        static uint32_t last_led_toggle_time = 0;
        static bool led_toggle = false;
        static uint32_t blink_interval = 1000;
        uint32_t now = System::GetNow();
        uint32_t elapsed_time = now - last_led_toggle_time;
        if (elapsed_time >= blink_interval) {
          // Alternate the LED lights in factory reset mode
          last_led_toggle_time = now;
          led_toggle = !led_toggle;
          led_left.Set(led_toggle ? 1.0f : 0.0f);
          led_right.Set(led_toggle ? 0.0f : 1.0f);
          led_left.Update();
          led_right.Update();
        }

        float low_knob_threshold = 0.05;
        float high_knob_threshold = 0.95;
        float blink_faster_amount = 300; // each stage removes this many MS from the factory reset blinking
        float knob_1_value = p_knob_1.Process();
        if (factory_reset_stage == 0 && knob_1_value >= high_knob_threshold) {
          factory_reset_stage++;
          blink_interval -= blink_faster_amount; // make the blinking faster as a UI feedback that the stage has been met
          quick_led_flash();          
        } else if (factory_reset_stage == 1 && knob_1_value <= low_knob_threshold) {
          factory_reset_stage++;
          blink_interval -= blink_faster_amount; // make the blinking faster as a UI feedback that the stage has been met
          quick_led_flash();          
        } else if (factory_reset_stage == 2 && knob_1_value >= high_knob_threshold) {
          factory_reset_stage++;
          blink_interval -= blink_faster_amount; // make the blinking faster as a UI feedback that the stage has been met
          quick_led_flash();          
        } else if (factory_reset_stage == 3 && knob_1_value <= low_knob_threshold) {
          SavedSettings.RestoreDefaults();
          load_settings();
          quick_led_flash();          

          hw.StartAudio(AudioCallback);
          factory_reset_stage = 0;
          is_factory_reset_mode = false;
        }
      // }
    }
    hw.DelayMs(10);

    // Toggle effect bypass LED when footswitch is pressed
    // led_bypass.Set(bypass ? 0.0f : 1.0f);
    // led_bypass.Update();

    // Call System::ResetToBootloader() if FOOTSWITCH_1 is pressed for 2 seconds
    hw.CheckResetToBootloader();
  }
  return 0;
}