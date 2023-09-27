#pragma once

#include "IPlug_include_in_plug_hdr.h"

#include "NonIntegerResampler.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kResamplerType,
  kNumParams
};

enum EResamplerType
{
  kLinear = 0,
  kCubic,
  kLanczos,
  kNumInterpTypes
};

using namespace iplug;
using namespace igraphics;

class IPlugSRC final : public Plugin
{
public:
  IPlugSRC(const InstanceInfo& info);

  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnParamChange(int paramIdx, EParamSource src, int sampleOffset) override;
  
  NonIntegerResampler<iplug::sample> mNonIntegerResampler {48000.0, ESRCMode::kLancsoz};
};
