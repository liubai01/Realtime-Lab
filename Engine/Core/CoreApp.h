#pragma once
#include "../Base/BaseApp.h"
#include <windows.h>
#include "../DebugOut.h"
#include "../Base/BaseApp.h"
#include "../Base/BaseGeometry.h"
#include "../Base/BaseUploadHeap.h"
#include "../Base/BaseDirectCommandList.h"
#include "CoreLightManager.h"
#include "CoreRenderTextureManager.h"
#include "Resource/CoreResourceManager.h"
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
    // --- Draw Context ---

    // The draw context used for rendering phong model of objects
    // *Remark*: right now we use the phong model
    unique_ptr<CoreDrawDiffuseContext> mDrawContext;

    // The edgelight draw context draws the mask of object
    // of interest
    unique_ptr<CoreDrawEdgeContext> mEdgeLightDrawContext;

    // The blur draw context blends final edge light with 
    // with the objects
    unique_ptr<CoreDrawBlurContext> mBlurDrawContext;

    // The Upload command list used for uploading geometries
    // from CPU to GPU
    unique_ptr<BaseDirectCommandList> mUploadCmdList;

    // The UI draw CommandList used for drawing Dear ImGUI
    // for the editor
    unique_ptr<BaseDirectCommandList> mUIDrawCmdList;

    // --- Managers ---

    // The light manager aggregates the light information from
    // the active light components
    unique_ptr<CoreLightManager> mLightManager;
    
    // The GUI manager is responsible for the editor GUI
    // based on Dear Im-GUI
    unique_ptr<CoreGUIManager> mGUIManager;

    // The render texture manager is responsible for the
    // render target used as a texture, all render textures
    // should be allocated from here
    unique_ptr<CoreRenderTextureManager> mRenderTextureManager;

    // All meshes should be loaded from the mesh loader
    // Remark: The BaseResourceMesh loaded from resource manager 
    //         could not be used directly in rendering
    unique_ptr<CoreMeshLoader> mMeshLoader;

    // Resource manager load files as handy handles from 
    // asset manager. 
    unique_ptr<CoreResourceManager> mResourceManager;

    // --- MISCS ---

    // The screen covers the whole screen is used for 
    // traverse each pixels of the screen when applying blurring
    unique_ptr<CoreMeshComponent> mFullScreenPlane;

    // The scene render texture hold final rendering objects
    shared_ptr<BaseRenderTexture> mSceneRenderTexture;
    // The edge render texture stores tempororary mask
    // of the object that user selected (shaded it with a bright edge)
    shared_ptr<BaseRenderTexture> mEdgeRenderTexture;
  
    CoreApp(HINSTANCE hInstance, BaseProject* proj);

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