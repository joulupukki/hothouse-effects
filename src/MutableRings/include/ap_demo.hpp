#pragma once

#include "common.hpp"
#include "fx_engine.hpp"
#include <ranges>

class AllPassDemo {
  constexpr static size_t max_excursion = 16;

 public:
  AllPassDemo(Buffer buffer) : engine_{buffer} {};
  ~AllPassDemo() = default;

  void Init(float sample_rate) {
    engine_.SetLFOFrequency(LFO_1, 0.5f / sample_rate);
    engine_.SetLFOFrequency(LFO_2, 0.3f / sample_rate);
  }

  void Process(StereoSignal in, StereoBuffer out) {
    typename FxEngine::Context c;

    typename FxEngine::AllPass ap1(4453 * size_);

    FxEngine::ConstructTopology(engine_, {&ap1});

    const float kid1 = diffusion_;  // input diffusion 1

    const float amount = amount_;

    for (auto&& [in_s, out_s] : std::views::zip(in, out)) {
      engine_.Advance();

      c.Set((in_s.left + in_s.right) * input_gain_);
      ap1.Process(c, kid1);
      const float out = c.Get();

      out_s.left += (out - in_s.left) * amount;
      out_s.right += (out - in_s.right) * amount;
    }
  }

  inline void set_amount(float amount) { amount_ = amount; }

  inline void set_input_gain(float input_gain) { input_gain_ = input_gain; }

  inline void set_diffusion(float diffusion) { diffusion_ = diffusion; }

  inline void set_size(float size) { size_ = size; }

 private:
  FxEngine engine_;

  float size_ = 1.f;
  float amount_ = 0.f;
  float input_gain_ = 1.f;
  float diffusion_ = 0.750f;
};
