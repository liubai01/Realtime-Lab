#pragma once
#include "../Base/BaseApp.h"
#include <windows.h>
#include "../DebugOut.h"
#include "../Base/BaseApp.h"
#include "../Base/BaseGeometry.h"
#include "../Base/BaseUploadHeap.h"
#include "../Base/BaseImage.h"
#include <memory>

#include "../ThirdParty/ImGUI/imgui.h"
#include "../ThirdParty/ImGUI/imgui_impl_win32.h"
#include "../ThirdParty/ImGUI/imgui_impl_dx12.h"


__declspec(align(16)) class CoreApp : public BaseApp
{
public:
  unique_ptr<BaseDrawContext> mDrawContext;

  CoreApp(HINSTANCE hInstance);

  void Start();
  void Update();
  void Render();

  void* operator new(size_t i)
  {
    return _mm_malloc(i, 16);
  }

  void operator delete(void* p)
  {
    _mm_free(p);
  }
};