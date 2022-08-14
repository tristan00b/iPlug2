/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

/**
 * @file
 * @ingroup Controls
 * @copydoc IPlatformViewControl
 */

#include "IControl.h"
#include "IPlugPlatformView.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/**
 * @ingroup IControls */
class IPlatformViewControl : public IControl, public IPlatformView
{
public:
  /** Constructs am IWebViewControl
   * @param bounds The control's bounds
   * @param opaque Should the web view background be opaque */
  IPlatformViewControl(const IRECT& bounds, bool opaque)
  : IControl(bounds)
  , IPlatformView(opaque)
  {
  }
  
  ~ISwiftViewControl()
  {
    GetUI()->RemovePlatformView(mPlatformView);
    mPlatformView = nullptr;
  }
  
  void OnAttached() override
  {
    mPlatformView = CreatePlatformView(GetUI()->GetWindow(), mRECT.L, mRECT.T, mRECT.W(), mRECT.H(), GetUI()->GetTotalScale());
    GetUI()->AttachPlatformView(mRECT, mPlatformView);
  }
  
  void Draw(IGraphics& g) override
  {
//    g.FillRect(COLOR_YELLOW, mRECT);
     /* NO-OP */
  }
  
  void OnMouseDown(float x, float y, const IMouseMod& mod) override
  {
  }

  void OnRescale() override
  {
    UpdateChildViewBounds();
  }

  void OnResize() override
  {
    UpdateChildViewBounds();
  }
  
private:
  void UpdateChildViewBounds()
  {
    auto ds = GetUI()->GetDrawScale();
    SetChildViewBounds(mRECT.L * ds, mRECT.T * ds, mRECT.W() * ds, mRECT.H() * ds, ds);
  }
  
  void* mPlatformView = nullptr;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE

