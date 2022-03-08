#pragma once

#include <vector>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <windows.h>
#include "d3dx12.h"
#include <wrl.h>


using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace std;



template <class T>
class Geometry
{
public:
  vector<T> mVertices;

  ComPtr<ID3D12Resource> mVertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
  D3D12_SUBRESOURCE_DATA mVertexData;
  ComPtr<ID3D12Resource> mvBufferUploadHeap;
};

