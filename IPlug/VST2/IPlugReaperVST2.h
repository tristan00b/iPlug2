/*
 ==============================================================================
 
 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers. 
 
 See LICENSE.txt for  more info.
 
 ==============================================================================
*/

#pragma once

#include "aeffect.h"

#include "IPlugVST2.h"
#include "reaper_plugin.h"
#include "video_processor.h"

#define REAPERAPI_IMPLEMENT
#include "reaper_plugin_functions.h"
#include "reaper_plugin_fx_embed.h"

bool (*DoFxLastTweakParmCtxMenu2)(void* pFXDSP, void* pHWND, int xpos, int ypos, const char* headerStr);

BEGIN_IPLUG_NAMESPACE

/** Reaper specific VST2.4 API base class for an IPlug plug-in. */
class IPlugReaperVST2 : public IPlugVST2
{
public:
  IPlugReaperVST2(const InstanceInfo& info, const Config& config)
  : IPlugVST2(info, config)
  {
    int errorCount = REAPERAPI_LoadAPI([this](const char* str) {
                                         return (void*) mHostCallback(NULL, 0xdeadbeef, 0xdeadf00d, 0, (void*) str, 0.0);
                                       });
    if (errorCount > 0)
      LogToReaper("some errors when loading reaper api functions\n");
  }
  
  /** Pop up Reaper's "Last touched FX" dialog, in order to set envelopes etc for the last touched parameter
   * @param pView platform view handle - in an IGraphics plug-in you can call IGraphics::GetWindow() to get this
   * @param xpos x position on the screen to popup the menu. NOTE: screen coordinates! Can convert mouse position in IControls with IGraphics::ClientToScreen()
   * @param ypos y position on the screen to popup the menu. NOTE: screen coordinates! Can convert mouse position in IControls with IGraphics::ClientToScreen() */
  void CreateParameterPopup(void* pView, int xpos, int ypos)
  {
    void *pFXDSP = (void*)mHostCallback(&mAEffect, 0xdeadbeef, 0xdeadf00e, 4, NULL, 0.0f);

    *(VstIntPtr *)&DoFxLastTweakParmCtxMenu2 = mHostCallback(NULL, 0xdeadbeef, 0xdeadf00d, 0, (void*) "DoFxLastTweakParmCtxMenu2", 0.0);
    
    if(DoFxLastTweakParmCtxMenu2)
      DoFxLastTweakParmCtxMenu2(pFXDSP, pView, xpos, ypos, NULL);
  }
  
  void SetTrackVolume(double gain)
  {
    MediaTrack* tr = GetReaperTrack();

    if (tr != nullptr)
      SetMediaTrackInfo_Value(tr, "D_VOL", gain);
  }
  
  void GetTakeName(WDL_String& str)
  {
    const char* name = ::GetTakeName(GetReaperTake());
    if (name != nullptr)
      str.Set(name);
  }

  void SetTakeName(const char* name)
  {
    MediaItem_Take* tk = GetReaperTake();
    if (tk != nullptr)
    {
      GetSetMediaItemTakeInfo_String(tk, "P_NAME", const_cast<char*>(name), true);
      UpdateArrange();
    }
    else
    {
      LogToReaper("Plugin is not loaded into a Reaper take\n");
    }
  }

  void GetTrackName(WDL_String& str) override
  {
    char buf[2048];
    if (GetSetMediaTrackInfo_String(GetReaperTrack(), "P_NAME", buf, false) == true)
    {
      str.Set(buf);
    }
  }

  void SetTrackName(const char* name)
  {
    GetSetMediaTrackInfo_String(GetReaperTrack(), "P_NAME", const_cast<char*>(name), true);
    UpdateArrange();
  }

  MediaTrack* GetReaperTrack()
  {
    return (MediaTrack*)mHostCallback(&mAEffect, 0xdeadbeef, 0xdeadf00e, 1, 0, 0.0);
  }

  MediaItem_Take* GetReaperTake()
  {
    return (MediaItem_Take*) mHostCallback(&mAEffect, 0xdeadbeef, 0xdeadf00e, 2, 0, 0.0);
  }
  
  void GetTrackColor(int& r, int& g, int& b) override
  {
    MediaTrack* pTrack = GetReaperTrack();
    
    ColorFromNative(::GetTrackColor(pTrack), &r, &g, &b);
  }
  
  void LogToReaper(const char* str)
  {
    if(ShowConsoleMsg != nullptr)
      ShowConsoleMsg(str);
  }
  
  void InformHostOfAddedParams(int index, int numAddedParams)
  {
    int listadj[2] = {index, numAddedParams};
    mAEffect.numParams = NParams();
    mHostCallback(&mAEffect, audioMasterVendorSpecific, 0xdeadbeef, audioMasterAutomate, listadj, 0.0);
  }
  
  void InformHostOfRemovedParams(int index, int numRemovedParams)
  {
    int listadj[2] = {index, -numRemovedParams};
    mAEffect.numParams = NParams();
    mHostCallback(&mAEffect, audioMasterVendorSpecific, 0xdeadbeef, audioMasterAutomate, listadj, 0.0);
  }
  
  void InitializeVideo(void* staticProcessVideoFrame, void* staticGetVideoParam)
  {
    void* pCtx = (void*) mHostCallback(&mAEffect, 0xdeadbeef, 0xdeadf00e, 4, NULL, 0.0f);
    
    if (pCtx)
    {
      IREAPERVideoProcessor *(*video_CreateVideoProcessor)(void *fxctx, int version);
      *(void**)&video_CreateVideoProcessor = (void *) mHostCallback(&mAEffect, 0xdeadbeef, 0xdeadf00d, 0, (void *) "video_CreateVideoProcessor", 0.0f);
      if (video_CreateVideoProcessor)
      {
        mVideoProc = video_CreateVideoProcessor(pCtx, IREAPERVideoProcessor::REAPER_VIDEO_PROCESSOR_VERSION);
        if (mVideoProc)
        {
          mVideoProc->userdata = this;
          mVideoProc->process_frame = (IVideoFrame* (*)(IREAPERVideoProcessor*, const double*, int, double, double, int)) staticProcessVideoFrame;
          mVideoProc->get_parameter_value = (bool (*)(IREAPERVideoProcessor*, int, double *)) staticGetVideoParam;
        }
      }
    }
  }
  
  virtual void GetEmbeddedUIPrefferedAspectRatio(int& numerator, int& denominator) { numerator = 1; denominator = 1; };
  virtual void GetEmbeddedUIMinimumAspectRatio(int& numerator, int& denominator) { numerator = 1; denominator = 1; };

  virtual void OnCreateEmbeddedUI() { /* No Op */ };
  virtual void OnDestroyEmbeddedUI() { /* No Op */ };
  virtual void OnEmbeddedUIMouseOver(int mouseX, int mouseY) { /* No Op */ }
  virtual void OnEmbeddedUIMouseLeft(int mouseX, int mouseY, bool down) { /* No Op */ }
  virtual void OnEmbeddedUIMouseRight(int mouseX, int mouseY, bool down) { /* No Op */ }
  virtual void OnEmbeddedUIResize(int w, int h) { /* No Op */ };
  virtual void DrawEmbeddedUI(REAPER_FXEMBED_IBitmap* pBitmap, int mouseX, int mouseY, bool leftMouseDown, bool rightMouseDown)
  {
    /* No Op */
  };

private:
  VstIntPtr VSTVendorSpecific(VstInt32 idx, VstIntPtr value, void* ptr, float opt) override
  {
    if(idx == AEffectOpcodes::__effEditDrawDeprecated)
    {
      int message = (int) opt;
      
      switch (message) {
        case 0: return 1;
        case WM_CREATE: OnCreateEmbeddedUI(); return 1;
        case WM_DESTROY: OnDestroyEmbeddedUI(); return 1;
        //case WM_SETCURSOR: return HCURSOR;
        case WM_MOUSEMOVE:
        {
          REAPER_inline_positioninfo* pInfo = reinterpret_cast<REAPER_inline_positioninfo*>(ptr);
          OnEmbeddedUIMouseOver(pInfo->mouse_x, pInfo->mouse_y);
          return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        {
          REAPER_inline_positioninfo* pInfo = reinterpret_cast<REAPER_inline_positioninfo*>(ptr);
          OnEmbeddedUIMouseLeft(pInfo->mouse_x, pInfo->mouse_y, message == WM_LBUTTONDOWN);
          return 0;
        }
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        {
          REAPER_inline_positioninfo* pInfo = reinterpret_cast<REAPER_inline_positioninfo*>(ptr);
          OnEmbeddedUIMouseRight(pInfo->mouse_x, pInfo->mouse_y, message == WM_RBUTTONDOWN);
          return 0;
        }
        case WM_GETMINMAXINFO:
        {
          MINMAXINFO* pInfo = reinterpret_cast<MINMAXINFO*>(ptr);
          int aspectNumerator;
          int aspectDenominator;
          GetEmbeddedUIPrefferedAspectRatio(aspectNumerator, aspectDenominator);
          pInfo->ptReserved.x = (aspectNumerator<<16)/aspectDenominator;
          GetEmbeddedUIMinimumAspectRatio(aspectNumerator, aspectDenominator);
          pInfo->ptReserved.y = (aspectNumerator<<16)/aspectDenominator;
          return 1;
        }
        case WM_PAINT:
        {
          REAPER_FXEMBED_IBitmap* pBitmap = FromVstPtr<REAPER_FXEMBED_IBitmap>(value);
          REAPER_inline_positioninfo* pInfo = reinterpret_cast<REAPER_inline_positioninfo*>(ptr);
          
          static int oldWidth = 0;
          static int oldHeight = 0;
          
          if(pInfo->width != oldWidth || pInfo->height != oldHeight)
          {
            OnEmbeddedUIResize(pInfo->width, pInfo->height);
            oldWidth = pInfo->width;
            oldHeight = pInfo->height;
          }
          
          int extraFlag = 0;
          if(pInfo->extraParms[0])
            extraFlag = (int)(INT_PTR) pInfo->extraParms[0];
          
//          if(extraFlag != 1)
          DrawEmbeddedUI(pBitmap, pInfo->mouse_x, pInfo->mouse_y, extraFlag & 0x10000, extraFlag & 0x20000);
          
          return 1;
        }
        default:
          return 0;
      }
    }
    return 0;
  }

  VstIntPtr VSTCanDo(const char* hostString) override
  {
    if (!strcmp(hostString, "hasCockosEmbeddedUI"))
    {
      return 0xbeef0000;
    }
    
    return 0;
  }
//  template<typename T>
//  T (*GetFunc(VstInt32 p1, VstIntPtr p2, const char* str = NULL, float p3 = 0.f))()
//  {
//    T (*func)();
//    *(long *)&func = mHostCallback(&mAEffect, 0xdeadbeef, p1, p2, (void*) str, p3);
//    return func;
//  }
  
  IREAPERVideoProcessor* mVideoProc = nullptr;
};

IPlugReaperVST2* MakePlug(const InstanceInfo& info);

END_IPLUG_NAMESPACE
