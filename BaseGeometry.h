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

  void ComputeNormal(vector<XMFLOAT3*>& pos, vector<XMFLOAT3*>& outs);
};

template <class T>
void BaseGeometry<T>::ComputeNormal(vector<XMFLOAT3*>& pos, vector<XMFLOAT3*>& outs)
{
  // Traverse all faces
  for (size_t i = 0; i < mIndices.size() / 3; ++i)
  {
    // indice of each triangle in this surface
    UINT i0 = mIndices[i * 3 + 0];
    UINT i1 = mIndices[i * 3 + 1];
    UINT i2 = mIndices[i * 3 + 2];

    // vertices of ith triangle
    T& v0 = mVertices[i0];
    T& v1 = mVertices[i1];
    T& v2 = mVertices[i2];

    // compute face normal
    XMVECTOR v0_Pos = XMLoadFloat3(pos[i0]);
    XMVECTOR v0_Normal = XMLoadFloat3(outs[i0]);
    XMVECTOR v1_Pos = XMLoadFloat3(pos[i1]);
    XMVECTOR v1_Normal = XMLoadFloat3(outs[i1]);
    XMVECTOR v2_Pos = XMLoadFloat3(pos[i2]);
    XMVECTOR v2_Normal = XMLoadFloat3(outs[i2]);

    XMVECTOR e0 = v1_Pos - v0_Pos;
    XMVECTOR e1 = v2_Pos - v0_Pos;
    XMVECTOR faceNormal = XMVector3Normalize(XMVector3Cross(e0, e1));

    v0_Normal += faceNormal;
    v1_Normal += faceNormal;
    v2_Normal += faceNormal;

    XMStoreFloat3(outs[i0], v0_Normal);
    XMStoreFloat3(outs[i1], v1_Normal);
    XMStoreFloat3(outs[i2], v2_Normal);
  }

  for (size_t i = 0; i < mVertices.size(); ++i)
  {
    XMVECTOR v_Normal = XMLoadFloat3(outs[i]);
    v_Normal = XMVector3Normalize(v_Normal);
    XMStoreFloat3(outs[i], v_Normal);
  }
}
