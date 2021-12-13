#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IPlugReaperPlugin_MiniEditor.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class IPlugReaperPlugin : public Plugin
{
public:
  IPlugReaperPlugin(const InstanceInfo& info);
  
  void DrawEmbeddedUI(REAPER_FXEMBED_IBitmap* pBitmap, int mouseX, int mouseY, bool leftMouseDown, bool rightMouseDown) override;
  void GetEmbeddedUIPrefferedAspectRatio(int& numerator, int& denominator) override { numerator = 4; denominator = 3; };
  void GetEmbeddedUIMinimumAspectRatio(int& numerator, int& denominator) override { numerator = 1; denominator = 1; };
  void OnCreateEmbeddedUI() override;
  void OnDestroyEmbeddedUI() override;
  void OnEmbeddedUIMouseOver(int x, int y) override;
  void OnEmbeddedUIMouseLeft(int x, int y, bool down) override;
  void OnEmbeddedUIMouseRight(int x, int y, bool down) override;
  void OnEmbeddedUIResize(int w, int h) override;
  
  void OnParamChangeUI(int paramIdx, EParamSource src) override;

#if IPLUG_DSP // All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
  
  MiniEditor mMiniEditor;
};
