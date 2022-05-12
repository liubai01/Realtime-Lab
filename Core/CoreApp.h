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
#include "CoreRenderTextureManager.h"
#include "GUI/CoreGUIManager.h"
#include "DrawContext/CoreDrawDiffuseContext.h"
#include "DrawContext/CoreDrawEdgeContext.h"
#include "DrawContext/CoreDrawBlurContext.h"
#include <memory>
#include "Component/CoreMeshComponent.h"
#include "CoreMeshLoader.h"

#include "../ThirdParty/ImGUI/imgui.h"
#include "../ThirdParty/ImGUI/imgui_impl_win32.h"
#include "../ThirdParty/ImGUI/imgui_impl_dx12.h"


__declspec(align(16)) class CoreApp : public BaseApp
{
public:
  unique_ptr<CoreDrawDiffuseContext> mDrawContext;
  unique_ptr<CoreDrawEdgeContext> mEdgeLightDrawContext;
  unique_ptr<CoreDrawBlurContext> mBlurDrawContext;
  unique_ptr<CoreMeshComponent> mFullScreenPlane;

  unique_ptr<BaseDirectCommandList> mUploadCmdList;
  unique_ptr<BaseDirectCommandList> mUIDrawCmdList;

  unique_ptr<CoreMaterialManager> mMaterialManager;
  unique_ptr<CoreLightManager> mLightManager;
  unique_ptr<CoreGUIManager> mGUIManager;
  unique_ptr<CoreRenderTextureManager> mRenderTextureManager;
  unique_ptr<CoreMeshLoader> mMeshLoader;

  shared_ptr<BaseRenderTexture> mSceneRenderTexture;
  shared_ptr<BaseRenderTexture> mEdgeRenderTexture;
  
  CoreApp(HINSTANCE hInstance);

  shared_ptr<CoreMaterial> CreateMaterial(const string& name);

  virtual void Start();
  virtual void Update();
  void BeforeUpdate();
  void Render();

  void UploadGeometry();
  void RenderObjects();
  void RenderEdgeLightPre();
  void RenderBlurred();
  void RenderUI();

  void* operator new(size_t i)
  {
    return _mm_malloc(i, 16);
  }

  void operator delete(void* p)
  {
    _mm_free(p);
  }
};