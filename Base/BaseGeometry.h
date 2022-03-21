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
using namespace std;


template <class T>
class BaseGeometry
{
public:
  // Name for debugging (set the name of resource heap, etc.)
  std::string mName = "Geometry";

  vector<T> mVertices = {};
  vector<DWORD> mIndices = {};

  //void ComputeNormal(vector<XMFLOAT3*>& pos, vector<XMFLOAT3*>& outs);
};
