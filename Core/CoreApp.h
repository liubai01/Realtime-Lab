#pragma once
#include "../Base/BaseApp.h"
#include <windows.h>
#include "../DebugOut.h"
#include "../Base/BaseApp.h"
#include "../Base/BaseGeometry.h"
#include "../Base/BaseUploadHeap.h"
#include "../Base/BaseImage.h"
#include "../Base/BaseDirectCommandList.h"
#include "CoreMaterialManager.h"
#include "CoreLightManager.h"
#include <memory>

#include "../ThirdParty/ImGUI/imgui.h"
#include "../ThirdParty/ImGUI/imgui_impl_win32.h"
#include "../ThirdParty/ImGUI/imgui_impl_dx12.h"


__declspec(align(16)) class CoreApp : public BaseApp
{
public:
  unique_ptr<BaseDrawContext> mDrawContext;
  unique_ptr<BaseDirectCommandList> mUploadCmdList;
  unique_ptr<CoreMaterialManager> mMaterialManager;
  unique_ptr<CoreLightManager> mLightManager;

  CoreApp(HINSTANCE hInstance);

  shared_ptr<CoreMaterial> CreateMaterial(const string& name);

  virtual void Start();
  virtual void Update();
  void Render();

  void UploadGeometry();
  void RenderObjects();

  void* operator new(size_t i)
  {
    return _mm_malloc(i, 16);
  }

  void operator delete(void* p)
  {
    _mm_free(p);
  }
};