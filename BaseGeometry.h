#pragma once

#include <vector>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <windows.h>
#include "d3dx12.h"
#include <wrl.h>
#include <string>
#include "DebugOut.h"


using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace std;


template <class T>
class BaseGeometry
{
public:
  // name
  std::string mName = "Geo";

  vector<T> mVertices;
  vector<DWORD> mIndices;
};
