#include "IPlugReaperPlugin_MiniEditor.h"
#include "config.h"
#include "IControls.h"
#include "lice.h"

using namespace iplug;
using namespace igraphics;

MiniEditor::MiniEditor(int numParams, IGEditorDelegate& mainEditor)
: IGEditorDelegate(numParams)
, mMainEditor(mainEditor)
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");
}

IGraphics* MiniEditor::CreateGraphics()
{
  IGraphics* pGraphics = MakeGraphics(*this, 300, 300, PLUG_FPS, 1.);
  
  pGraphics->EnableMouseOver(true);
  pGraphics->SetLayoutOnResize(true);
//  pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);

  
  return pGraphics;
}

void MiniEditor::LayoutUI(IGraphics* pGraphics)
{
  const IRECT b = pGraphics->GetBounds();

  if(pGraphics->NControls()) {
    pGraphics->GetBackgroundControl()->SetTargetAndDrawRECTs(b);
//    pGraphics->GetControlWithTag(0)->SetTargetAndDrawRECTs(b.GetPadded(-20.f));
    return;
  }

  pGraphics->AttachPanelBackground(COLOR_ORANGE);
//  pGraphics->AttachControl(new IVSliderControl(b.GetPadded(-20.f), kGain), 0);
//  pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain), 0);

//  pGraphics->AttachControl(new ILambdaControl(b.GetPadded(-20.f), [](ILambdaControl* pCaller, IGraphics& g, IRECT& r) {
//
////      const float width = 5.f;
//     const float radius = r.W();
////      const float cornerSize = 10.f;
//
//    //    g.FillRect(COLOR_WHITE, r);
//    //    g.FillCircle(COLOR_WHITE, r.MW(), r.MH(), radius);
//    //    g.FillArc(COLOR_WHITE, r.MW(), r.MH(), radius, 0, 90);
//    //    g.FillRoundRect(COLOR_WHITE, r, cornerSize);
//
//    //    g.DrawDottedLine(COLOR_WHITE, r.L, r.T, r.R, r.MH());
//    //    g.DrawRect(COLOR_WHITE, r, nullptr, width);
//    //    g.DrawCircle(COLOR_WHITE, r.MW(), r.MH(), radius, nullptr, width);
//    //    g.DrawArc(COLOR_WHITE, r.MW(), r.MH(), radius, 0, 90);
//    //    g.DrawRoundRect(COLOR_BLUE, r, cornerSize, nullptr, width);
//
//    const float x = r.MW();
//    const float y = r.MH();
//    const float rotate = static_cast<float>(pCaller->GetAnimationProgress() * PI);
//
//    for(int index = 0, limit = 40; index < limit; ++index)
//    {
//      float firstAngle = (static_cast<float>(index) * 2.f * static_cast<float>(PI)) / limit;
//      float secondAngle = ((static_cast<float>(index) + 1) * 2.f * static_cast<float>(PI)) / limit;
//
//      g.PathTriangle(x, y,
//                     x + std::sin(firstAngle + rotate) * radius, y + std::cos(firstAngle + rotate) * radius,
//                     x + std::sin(secondAngle + rotate) * radius, y + std::cos(secondAngle + rotate) * radius);
//
//      if(index % 2)
//        g.PathFill(COLOR_RED);
//      else
//        g.PathFill(COLOR_BLUE);
//    }
//
//
//  }, 1000, true, true), 0);
  
//    pGraphics->AttachControl(new IVButtonControl(b.GetPadded(-20.f), [](IControl* pCaller) {
//      WDL_String file;
//      WDL_String path;
//      pCaller->GetUI()->PromptForFile(file, path);
//    }), 0);
}

void MiniEditor::UpdateParamFromPlug(int paramIdx, double value)
{
  GetParam(paramIdx)->Set(value);
  SendParameterValueFromDelegate(paramIdx, value, false);
}

void MiniEditor::BeginInformHostOfParamChangeFromUI(int paramIdx)
{
  mMainEditor.BeginInformHostOfParamChangeFromUI(paramIdx);
}

void MiniEditor::SendParameterValueFromUI(int paramIdx, double normalizedValue)
{
  mMainEditor.SendParameterValueFromUI(paramIdx, normalizedValue);
  mMainEditor.IEditorDelegate::SendParameterValueFromDelegate(paramIdx, normalizedValue, true);
  IGEditorDelegate::SendParameterValueFromUI(paramIdx, normalizedValue);
}

void MiniEditor::EndInformHostOfParamChangeFromUI(int paramIdx)
{
  mMainEditor.EndInformHostOfParamChangeFromUI(paramIdx);
}
  
