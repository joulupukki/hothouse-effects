/**
Copyright (c) 2024 Boyd Timothy. All rights reserved.

Use of this source code is governed by the LGPL V2.1 license that can be found
in the LICENSE file or at https://opensource.org/license/lgpl-2-1/

Adapted from the Sploodge Reverb originally found here:
https://github.com/wgd-modular/loewenzahnhonig-firmware/blob/main/src/splooge-reverb/sploodge-reverb.cpp

SPDX-License-Identifier: LGPL-2.1
*/
#include "reverbsploodge.h"

#include "daisysp-lgpl.h"
#include "daisysp.h"
#include "hothouse.h"

#define REVSPLOODGE_OK 0
#define REVSPLOODGE_NOT_OK 1

using namespace daisy;
using namespace daisysp;

static PitchShifter pitch_shifter;
static Chorus chorus;
static ReverbSc DSY_SDRAM_BSS verb;
#define MAX_DELAY static_cast<size_t>(48000 * 2.5f)
static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS dell;
static DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delr;
static Svf filtL;
static Svf filtR;
static Balance balL;
static Balance balR;

// Init vars
float dryAmplitude, wetAmplitude;
float delayTimeSecs, delayOutL, delayOutR, delayFeedback;
float dryL, dryR, verbL, verbR, chorusL, chorusR, pitchShifter1, pitchShifter2,
    pitchShifterLR;
float reverbLpFreqControl, reverbLpFreq;
float dryWetMixL, dryWetMixR;

// Function to set value n to within the lower and upper limits
float clamp(float n, float lower, float upper) {
  return n <= lower ? lower : n >= upper ? upper : n;
}

int ReverbSploodge::Init(float sample_rate)
{
    sample_rate_   = sample_rate;
    feedback_      = 0.97;
    dryWet_        = 0.5;
    wetTone_       = 0.5;
    sploodge_      = 0.5;

    // Init DSP
    pitch_shifter.Init(sample_rate);
    chorus.Init(sample_rate);
    verb.Init(sample_rate);
    dell.Init();
    delr.Init();
    filtL.Init(sample_rate);
    filtR.Init(sample_rate);
    balL.Init(sample_rate);
    balR.Init(sample_rate);

    // Set delay time
    delayTimeSecs = sample_rate * 0.15f;
    dell.SetDelay(delayTimeSecs);
    delr.SetDelay(delayTimeSecs - 0.01f);

    // Set chorus params
    chorus.SetLfoFreq(0.33f, 0.2f);
    chorus.SetDelay(0.75f, 0.9f);
    chorus.SetLfoDepth(1.0f, 1.0f);
    chorus.SetFeedback(0.75f, 0.75f);

    init_done_     = 1;
    return REVSPLOODGE_OK;
}

int ReverbSploodge::Process(const float &in1, const float &in2, float *out1, float *out2)
{
    if (!init_done_) {
        return REVSPLOODGE_NOT_OK;
    }

    wetAmplitude = dryWet_;
    dryAmplitude = 1.0f - wetAmplitude;

    // Update reverb params
    verb.SetFeedback(0.1 + (0.8 * feedback_));
    reverbLpFreqControl = wetTone_;
    const float reverbLpFreqMin = log(100.0f);
    const float reverbLpFreqMax = log(28000.0f);
    reverbLpFreq = exp(reverbLpFreqMin + (reverbLpFreqControl * 
                                          (reverbLpFreqMax - reverbLpFreqMin)));
    verb.SetLpFreq(reverbLpFreq);

    // Set Filter params
    filtL.SetFreq(500.0 * wetTone_);
    filtL.SetRes(0.1);
    filtL.SetDrive(0.6);
    filtR.SetFreq(500.0 * wetTone_);
    filtR.SetRes(0.1);
    filtR.SetDrive(0.6);

    // Read in Audio Samples from left and right channels
    dryL = in1;
    dryR = in2;

    // Bounce the inputs down to mono for DSP functions
    float monoInput = (dryL * 0.7) + (dryR * 0.7);

    // Pitch-shift the input
    pitch_shifter.SetTransposition(24.0f);
    pitchShifter1 = pitch_shifter.Process(monoInput);
    pitch_shifter.SetTransposition(12.0f);
    pitchShifter2 = pitch_shifter.Process(monoInput);
    pitchShifterLR = (pitchShifter1 * (0.5 * (sploodge_ * sploodge_))) +
                     (pitchShifter2 * (0.2 * (sploodge_ * sploodge_)));

    // Push the pitch-shifted input through the chorus effect
    chorus.Process(pitchShifterLR);
    chorusL = chorus.GetLeft();
    chorusR = chorus.GetRight();

    // Run through a stereo delay line
    delr.SetDelay(delayTimeSecs);
    dell.SetDelay(delayTimeSecs);
    delayOutL = dell.Read();
    delayOutR = delr.Read();
    delayFeedback = (sploodge_ * 0.91);
    dell.Write((delayFeedback * delayOutL) + (dryL + (chorusL * 0.5)));
    delayOutL = (delayFeedback * delayOutL) +
                ((1.0f - delayFeedback) * (dryL + (chorusL * 0.5)));
    delr.Write((delayFeedback * delayOutR) + (dryL + (chorusR * 0.5)));
    delayOutR = (delayFeedback * delayOutR) +
                ((1.0f - delayFeedback) * (dryR + (chorusR * 0.5)));

    // Add reverb
    verb.Process((dryL * 0.5) + (delayOutL * sploodge_),
                 (dryL * 0.5) + (delayOutR * sploodge_), &verbL, &verbR);

    // Filter the final output for the Tone control
    filtL.Process(verbL * wetAmplitude);
    filtR.Process(verbR * wetAmplitude);

    dryWetMixL = (dryL * dryAmplitude) + filtL.High();
    dryWetMixR = (dryR * dryAmplitude) + filtR.High();

    // Apply make up gain to the wet signal if needed.
    // Not sure if this is even doing anything. Shouldn't the output of
    // balX.Process be assigned to dryWetMixX?
    balL.Process(dryWetMixL, dryL);
    balR.Process(dryWetMixR, dryR);

    *out1 = dryWetMixL;
    *out2 = dryWetMixR;

    return REVSPLOODGE_OK;
}