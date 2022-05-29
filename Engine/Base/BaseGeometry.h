#pragma once

#include <vector>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include <string>
#include "../DebugOut.h"


using namespace DirectX;
using Microsoft::WRL::ComPtr;


template <class T>
class BaseGeometry
{
public:
  // Name for debugging (set the name of resource heap, etc.)
  std::string mName = "Geometry";

  std::vector<T> mVertices = {};
  std::vector<DWORD> mIndices = {};
};
