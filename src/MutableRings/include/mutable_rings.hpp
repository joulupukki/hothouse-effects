// Copyright 2023 Katherine Whitlock
// Copyright 2014 Emilie Gillet
// Reverb.

#pragma once

#include <ranges>

#include "common.hpp"
#include "fx_engine.hpp"


class MutableRings {
 public:
  MutableRings(Buffer buffer) : engine_{buffer} {};
  ~MutableRings() = default;

  void Init(float sample_rate) {
    engine_.SetLFOFrequency(LFO_1, 0.5f / sample_rate);
    engine_.SetLFOFrequency(LFO_2, 0.3f / sample_rate);
    lp_ = 0.7f;
    diffusion_ = 0.625f;
  }

  void Process(StereoSignal in, StereoBuffer out) {
    // This is the Griesinger topology described in the Dattorro paper
    // (4 AP diffusers on the input, then a loop of 2x 2AP+1Delay).
    // Modulation is applied in the loop of the first diffuser AP for additional
    // smearing; and to the two long delays for a slow shimmer/chorus effect.
    typename FxEngine::AllPass ap1(150);
    typename FxEngine::AllPass ap2(214);
    typename FxEngine::AllPass ap3(319);
    typename FxEngine::AllPass ap4(527);

    typename FxEngine::AllPass dap1a(2182);
    typename FxEngine::AllPass dap1b(2690);
    typename FxEngine::AllPass del1(4501);

    typename FxEngine::AllPass dap2a(2525);
    typename FxEngine::AllPass dap2b(2197);
    typename FxEngine::AllPass del2(6312);

    typename FxEngine::Context c;
    FxEngine::ConstructTopology(engine_,  //<
                                {
                                    &ap1, &ap2, &ap3, &ap4,  //<
                                    &dap1a, &dap1b, &del1,   //<
                                    &dap2a, &dap2b, &del2,   //<
                                });

    const float kap = diffusion_;
    const float klp = lp_;
    const float krt = reverb_time_;
    const float amount = amount_;
    const float gain = input_gain_;

    float lp_1 = lp_decay_1_;
    float lp_2 = lp_decay_2_;

    for (auto&& [in_s, out_s] : std::views::zip(in, out)) {
      float wet = 0;
      float apout = 0.0f;
      engine_.Advance();

      // Smear AP1 inside the loop.
      // c.Interpolate(ap1, 10.0f, LFO_1, 80.0f, 1.0f);
      // c.Write(ap1, 100, 0.0f);

      c.Set((in_s.left + in_s.right) * gain);

      // Diffuse through 4 allpasses.
      ap1.Process(c, kap);
      ap2.Process(c, kap);
      ap3.Process(c, kap);
      ap4.Process(c, kap);
      apout = c.Get();

      // Main reverb loop.
      c.Set(apout);
      del2.Interpolate(c, 6261.0f, LFO_2, 50.0f, krt);
      c.Lp(lp_1, klp);
      dap1a.Process(c, -kap);
      dap1b.Process(c, kap);
      del1.Write(c, 2.0f);
      wet = c.Get();

      out_s.left += (wet - in_s.left) * amount;

      c.Set(apout);
      del1.Interpolate(c, 4460.0f, LFO_1, 40.0f, krt);
      c.Lp(lp_2, klp);
      dap2a.Process(c, -kap);
      dap2b.Process(c, kap);
      del2.Write(c, 2.0f);
      wet = c.Get();

      out_s.right += (wet - in_s.right) * amount;
    }

    lp_decay_1_ = lp_1;
    lp_decay_2_ = lp_2;
  }

  inline void set_amount(float amount) { amount_ = amount; }

  inline void set_input_gain(float input_gain) { input_gain_ = input_gain; }

  inline void set_time(float reverb_time) { reverb_time_ = reverb_time; }

  inline void set_diffusion(float diffusion) { diffusion_ = diffusion; }

  inline void set_lp(float lp) { lp_ = lp; }

  inline void Clear() { engine_.Clear(); }

 private:
  FxEngine engine_;

  float amount_;
  float input_gain_;
  float reverb_time_;
  float diffusion_;
  float lp_;

  float lp_decay_1_;
  float lp_decay_2_;
};
