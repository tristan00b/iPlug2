#include "IPlugEmbedSwift.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlatformViewControl.h"

IPlugEmbedSwift::IPlugEmbedSwift(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
//    GetScreenDimensions(width, height);
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.0f);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    const IRECT b = pGraphics->GetBounds();

    if (pGraphics->NControls()) {
      pGraphics->GetBackgroundControl()->SetTargetAndDrawRECTs(b);
      pGraphics->GetControl(1)->SetTargetAndDrawRECTs(b.GetCentredInside(100).GetHShifted(-100));
      return;
    }
    
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GREEN);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->SetLayoutOnResize(true);
    pGraphics->AttachControl(new IPlatformViewControl(b.GetCentredInside(100).GetHShifted(-100), false));
    pGraphics->AttachControl(new IPlatformViewControl(b.GetCentredInside(100).GetHShifted(-200), false));
    pGraphics->AttachControl(new IPlatformViewControl(b.GetCentredInside(100).GetHShifted(-300), false));

  };
#endif
}

void IPlugEmbedSwift::OnParentWindowResize(int width, int height)
{
//  if (GetUI())
//    GetUI()->Resize(width, height, 1.f, false);
}

#if IPLUG_DSP
void IPlugEmbedSwift::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif
