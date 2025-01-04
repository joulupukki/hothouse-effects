#pragma once

#include <ranges>

#include "common.hpp"
#include "fx_engine.hpp"


class DatorroPlate {
  constexpr static size_t max_excursion = 16;

 public:
  DatorroPlate(Buffer buffer) : engine_{buffer} {};
  ~DatorroPlate() = default;

  void Init(float sample_rate) {
    engine_.SetLFOFrequency(LFO_1, 0.5f / sample_rate);
    engine_.SetLFOFrequency(LFO_2, 0.3f / sample_rate);
  }

  void Process(StereoSignal in, StereoBuffer out) {
    typename FxEngine::Context c;

    typename FxEngine::AllPass ap1(142);
    typename FxEngine::AllPass ap2(107);
    typename FxEngine::AllPass ap3(379);
    typename FxEngine::AllPass ap4(277);

    typename FxEngine::AllPass dap1a(672 + max_excursion);
    typename FxEngine::DelayLine del1a(4453);
    typename FxEngine::AllPass dap1b(1800);
    typename FxEngine::DelayLine del1b(3720);

    typename FxEngine::AllPass dap2a(908 + max_excursion);
    typename FxEngine::DelayLine del2a(4217);
    typename FxEngine::AllPass dap2b(2656);
    typename FxEngine::DelayLine del2b(3163);

    FxEngine::ConstructTopology(
        engine_, {&ap1, &ap2, &ap3, &ap4, &dap1a, &del1a, &dap1b, &del1b,
                  &dap2a, &del2a, &dap2b, &del2b});

    const float kdecay = reverb_time_;  // 0.5f
    const float kid1 = 0.750f;          // input diffusion 1
    const float kid2 = 0.625f;          // input diffusion 2
    const float kdd1 = 0.70f;           // decay diffusion 1
    const float kdd2 =
        std::clamp(kdecay + 0.15f, 0.25f, 0.5f);  // decay diffusion 2

    const float kdamp = lp_;  // 1.f - 0.0005f;            // damping
    const float kbandwidth = 0.9995f;

    const float amount = amount_;
    const float gain = input_gain_;

    float lp_1 = lp_decay_1_;
    float lp_2 = lp_decay_2_;
    float lp_band = lp_band_;

    for (auto&& [in_s, out_s] : std::views::zip(in, out)) {
      engine_.Advance();

      c.Set((in_s.left + in_s.right) * gain);

      c.Lp(lp_band, kbandwidth);

      // Diffuse through 4 allpasses.
      ap1.Process(c, kid1);
      ap2.Process(c, kid1);
      ap3.Process(c, kid2);
      ap4.Process(c, kid2);
      float apout = c.Get();

      // Main reverb loop.
      c.Set(apout);
      dap1a.Interpolate(c, 672.0f, LFO_2, max_excursion, -kdd1);
      del1a.Process(c);
      c.Lp(lp_1, kdamp);  // damping
      c.Multiply(kdecay);
      dap1b.Process(c, kdd2);
      del1b.Process(c);
      c.Multiply(kdecay);
      c.Add(apout);
      dap2a.Write(c, kdd2);

      c.Set(apout);
      dap2a.Interpolate(c, 908.0f, LFO_1, max_excursion, -kdd1);
      del2a.Process(c);
      c.Lp(lp_1, kdamp);  // damping
      c.Multiply(kdecay);
      dap2b.Process(c, kdd2);
      del2b.Process(c);
      c.Multiply(kdecay);
      c.Add(apout);
      dap1a.Write(c, kdd1);

      float left_sum = 0;
      left_sum += 0.6f * del2a.at(266);
      left_sum += 0.6f * del2a.at(2974);
      left_sum -= 0.6f * dap2b.at(1913);
      left_sum += 0.6f * del2b.at(1996);
      left_sum -= 0.6f * del1a.at(1990);
      left_sum -= 0.6f * dap1b.at(187);
      left_sum -= 0.6f * del1b.at(1066);

      out_s.left += (left_sum - in_s.left) * amount;

      float right_sum = 0;
      right_sum += 0.6f * del1a.at(353);
      right_sum += 0.6f * del1a.at(3627);
      right_sum -= 0.6f * dap1b.at(1228);
      right_sum += 0.6f * del1b.at(2673);
      right_sum -= 0.6f * del2a.at(2111);
      right_sum -= 0.6f * dap2b.at(335);
      right_sum -= 0.6f * del2b.at(121);

      out_s.right += (right_sum - in_s.right) * amount;
    }

    lp_decay_1_ = lp_1;
    lp_decay_2_ = lp_2;
    lp_band_ = lp_band;
  }

  inline void set_amount(float amount) { amount_ = amount; }

  inline void set_input_gain(float input_gain) { input_gain_ = input_gain; }

  inline void set_time(float reverb_time) { reverb_time_ = reverb_time; }

  inline void set_diffusion(float diffusion) { diffusion_ = diffusion; }

  inline void set_lp(float lp) { lp_ = lp; }

  inline void Clear() { engine_.Clear(); }

 private:
  FxEngine engine_;

  float amount_ = 0.f;
  float input_gain_ = 1.f;
  float reverb_time_;
  float diffusion_;
  float lp_;
  float decay_ = 1.f;

  float lp_decay_1_;
  float lp_decay_2_;
  float lp_band_;
};
