#include "IPlugSRC.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"


IPlugSRC::IPlugSRC(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
  GetParam(kResamplerType)->InitEnum("Resampler", 0, {"linear", "cubic", "lanczos"});
}

void IPlugSRC::OnReset()
{
  mNonIntegerResampler.Reset(GetSampleRate());
}

void IPlugSRC::OnParamChange(int paramIdx, EParamSource src, int sampleOffset)
{
  if (paramIdx == kResamplerType)
    mNonIntegerResampler.SetResamplingMode((ESRCMode) GetParam(paramIdx)->Int());
}

void IPlugSRC::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const auto gain = GetParam(kGain)->GetNormalized();
  mNonIntegerResampler.ProcessBlock(inputs, outputs, nFrames, [&](sample** newInputs, sample** newOutputs, int newNFrames) {
    for (auto s=0; s<newNFrames; s++) {
      newOutputs[0][s] = newInputs[0][s] * gain;
      newOutputs[1][s] = newInputs[1][s] * gain;
    }
  });
}
