#pragma once
#include <span>

struct StereoSample {
  float left;
  float right;
};

using Buffer = std::span<float>;
using StereoBuffer = std::span<StereoSample>;
using StereoSignal = std::span<const StereoSample>;
