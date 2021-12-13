#include "IPlugReaperPlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlugReaperPlugin_MiniEditor.cpp"

IPlugReaperPlugin::IPlugReaperPlugin(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
, mMiniEditor(kNumParams, *this)
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
  };
#endif
}

void IPlugReaperPlugin::OnCreateEmbeddedUI()
{
  mMiniEditor.OpenWindow(nullptr);
}

void IPlugReaperPlugin::OnDestroyEmbeddedUI()
{
  mMiniEditor.CloseWindow();
}

void IPlugReaperPlugin::OnEmbeddedUIMouseOver(int x, int y)
{
  IMouseMod mod;
  mMiniEditor.GetUI()->OnMouseOver(x, y, mod);
}

void IPlugReaperPlugin::OnEmbeddedUIMouseLeft(int x, int y, bool down)
{
  IMouseInfo info;
  info.x = x;
  info.y = y;
  info.ms.L = true;
  std::vector<IMouseInfo> list {info};
  if(down)
    mMiniEditor.GetUI()->OnMouseDown(list);
  else
    mMiniEditor.GetUI()->OnMouseUp(list);
}

void IPlugReaperPlugin::OnEmbeddedUIMouseRight(int x, int y, bool down)
{
  IMouseInfo info;
  info.x = x;
  info.y = y;
  info.ms.R = true;
  std::vector<IMouseInfo> list {info};

  if(down)
    mMiniEditor.GetUI()->OnMouseDown(list);
  else
    mMiniEditor.GetUI()->OnMouseUp(list);
}

void IPlugReaperPlugin::OnEmbeddedUIResize(int w, int h)
{
  IGraphics* pGraphics = mMiniEditor.GetUI();
  
  if(pGraphics)
    pGraphics->Resize(w, h, 1.f);
}

void IPlugReaperPlugin::DrawEmbeddedUI(REAPER_FXEMBED_IBitmap* pBitmap, int mouseX, int mouseY, bool leftMouseDown, bool rightMouseDown)
{
  LICE_FillRect((LICE_IBitmap*) pBitmap, 0, 0, pBitmap->getWidth(), pBitmap->getHeight(), LICE_RGBA(255, 255, 255, 255), 1.f, 0);
  
  if(leftMouseDown || rightMouseDown)
    LICE_FillCircle((LICE_IBitmap*) pBitmap, mouseX, mouseY, 5.f, leftMouseDown ? LICE_RGBA(255, 0, 0, 255) : LICE_RGBA(0, 255, 0, 255), 1.f, 0, true);
  
  // TODO: BLIT IGraphics Mini Editor into pBitmap
//  LICE_FillRect(pBitmap, 0, 0, pBitmap->getWidth(), pBitmap->getHeight(), LICE_RGBA(255,0,0,255), 1.f, 0);
  
//  IGraphicsSkia* pGraphicsSkia = dynamic_cast<IGraphicsSkia*>(mMiniEditor.GetUI());
//
//  IMouseMod mod;
//  mod.L = leftMouseDown;
//  mod.R = rightMouseDown;
//
//  if(leftMouseDown || rightMouseDown)
//  {
//    static int lastX = 0;
//    static int lastY = 0;
//    pGraphicsSkia->OnMouseDrag(mouseX, mouseY, mouseX-lastX, mouseY-lastY, mod);
//    lastX = mouseX;
//    lastY = mouseY;
//  }
//
//  IRECTList rects;
//
//  pGraphicsSkia->SetAllControlsDirty();
//
//  if (pGraphicsSkia->IsDirty(rects))
//  {
//    pGraphicsSkia->SetAllControlsClean();
//    pGraphicsSkia->Draw(rects);
//  }
//
//  SkPixmap pixmap;
//  if(pGraphicsSkia->mSurface->peekPixels(&pixmap))
//  {
//    int w = pixmap.bounds().width();
//    int h = pixmap.bounds().height();
//
//    for(int y = 0; y < h; y ++)
//    {
//      for(int x = 0; x < w; x++)
//      {
//        const uint8_t* pSkiaPixel = reinterpret_cast<const uint8_t*>(pixmap.addr32(x, y));
//        LICE_PutPixel(pBitmap, x, y, LICE_RGBA(pSkiaPixel[0], pSkiaPixel[1], pSkiaPixel[2], 255), 1.f, 0); //
//      }
//    }
//    //TODO: issue with alpha (premultiplied in skia?) when using LICE_Blit
////    auto liceSrc = LICE_WrapperBitmap(reinterpret_cast<LICE_pixel*>(pixmap.writable_addr()), w, h, (int) pixmap.rowBytesAsPixels(), false);
////    LICE_Blit(pBitmap, &liceSrc, 0, 0, 0, 0, w, h, 1.f, LICE_BLIT_MODE_COPY);
//  }
}

void IPlugReaperPlugin::OnParamChangeUI(int paramIdx, EParamSource src)
{
  mMiniEditor.UpdateParamFromPlug(paramIdx, GetParam(paramIdx)->Value());
//  mMiniEditor.OnParamChange(paramIdx);
//  mMiniEditor.OnParamChangeUI(paramIdx, src);
}

#if IPLUG_DSP
void IPlugReaperPlugin::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
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
