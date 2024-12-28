/**
Copyright (c) 2024 Boyd Timothy. All rights reserved.

Use of this source code is governed by the LGPL V2.1 license that can be found
in the LICENSE file or at https://opensource.org/license/lgpl-2-1/

Adapted from the Sploodge Reverb originally found here:
https://github.com/wgd-modular/loewenzahnhonig-firmware/blob/main/src/splooge-reverb/sploodge-reverb.cpp

SPDX-License-Identifier: LGPL-2.1
*/
#pragma once
#ifndef REVERB_SPLOODGE_H
#define REVERB_SPLOODGE_H

namespace daisysp {
class ReverbSploodge {
public:
    ReverbSploodge() {}
    ~ReverbSploodge() {}

    /// @brief Initializes the reverb module, and sets the sample_rate at which
    /// the Process function will be called.
    /// @param sample_rate The sample rate that the Process function will be
    /// called at.
    /// @return Returns 0 if all good, or 1 if it runs out of delay times exceed
    /// maximum allowed.
    int Init(float sample_rate);

    /// @brief Process the input through the reverb, and updates values of out1,
    /// and out2 with the new processed signal.
    int Process(const float &in1, const float &in2, float *out1, float *out2);

    /// @brief controls the reverb time.
    /// reverb tail becomes infinite when set to 1.0
    /// @param fb sets reverb time. Range: 0.0 to 1.0
    inline void SetFeedback(const float &fb) { feedback_ = fb; }

    /// @brief controls the balance between dry and wet signal.
    /// @param fb Sets the amount of wet signal. Range: 0.0 to 1.0
    inline void SetDryWet(const float &dryWet) { dryWet_ = dryWet; }

    /// @brief controls the tone of the wet signal.
    /// @param wetTone Sets the tone of the wet signal. Range: 0.0 to 1.0
    inline void SetWetTone(const float &wetTone) { wetTone_ = wetTone; }

    /// @brief controls the amount of "Sploodge".
    /// @param sploodge Sets the amount of sploodge. Range: 0.0 to 1.0
    inline void SetSploodge(const float &sploodge) { sploodge_ = sploodge; }

private:
    float   sample_rate_;

    // Tweakable params at runtime:
    float   feedback_;
    float   dryWet_;
    float   wetTone_;
    float   sploodge_;

    int     init_done_;
};

} // namespace daisysp
#endif // REVERB_SPLOODGE_H