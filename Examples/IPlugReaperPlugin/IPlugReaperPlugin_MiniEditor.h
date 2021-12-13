#pragma once

#include "IGraphics_include_in_plug_hdr.h"

using namespace iplug;
using namespace igraphics;

class MiniEditor : public IGEditorDelegate
{
public:
  MiniEditor(int numParams, IGEditorDelegate& mainEditor);
  
  IGraphics* CreateGraphics() override;
  
  void LayoutUI(IGraphics* pGraphics) override;
  
  void BeginInformHostOfParamChangeFromUI(int paramIdx) override;
  void SendParameterValueFromUI(int paramIdx, double normalizedValue) override;
  void EndInformHostOfParamChangeFromUI(int paramIdx) override;

  void UpdateParamFromPlug(int paramIdx, double value);
private:
  IGEditorDelegate& mMainEditor;
};
