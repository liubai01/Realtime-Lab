#pragma once
#include <windows.h>
#include "DebugOut.h"
#include "BaseApp.h"
#include "BaseGeometry.h"
#include "BaseUploadHeap.h"
#include <memory>

using namespace std;

struct Vertex {
  XMFLOAT3 pos;
  XMFLOAT4 color;
  XMFLOAT3 normal;
};

void SetCubeGeo(BaseGeometry<Vertex>& geo, XMFLOAT4& color)
{
  geo.mVertices = {
    // front
    { { -1.0f, +1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, -1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, -1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, +1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    // right
    { { +1.0f, -1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, +1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, -1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, +1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    // left
    { { -1.0f, +1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, -1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, -1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, +1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    // back
    { { +1.0f, +1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, -1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, -1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, +1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    // top
    { { -1.0f, +1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, +1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, +1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, +1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    // bottom
    { { +1.0f, -1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, -1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { +1.0f, -1.0f, -1.0f }, color, { 0.0f, 0.0f, 0.0f } },
    { { -1.0f, -1.0f, +1.0f }, color, { 0.0f, 0.0f, 0.0f } },
  };
  geo.mIndices = {
    // ffront face
    0, 1, 2, // first triangle
    0, 3, 1, // second triangle

    // left face
    4, 5, 6, // first triangle
    4, 7, 5, // second triangle

    // right face
    8, 9, 10, // first triangle
    8, 11, 9, // second triangle

    // back face
    12, 13, 14, // first triangle
    12, 15, 13, // second triangle

    // top face
    16, 17, 18, // first triangle
    16, 19, 17, // second triangle

    // bottom face
    20, 21, 22, // first triangle
    20, 23, 21, // second triangle
  };

  vector<XMFLOAT3*> norms;
  vector<XMFLOAT3*> pos;

  for (auto& v : geo.mVertices) {
    pos.push_back(&v.pos);
    norms.push_back(&v.normal);
  }

  geo.ComputeNormal(pos, norms);
}

class MyApp : public BaseApp {
public:
  vector<Vertex> mvList;
  MyApp(HINSTANCE hInstance);

  void Render();
  void Update();

#pragma region [Constant Buffer]

  ComPtr<ID3D12DescriptorHeap> mConstDescHeap;
  D3D12_DESCRIPTOR_RANGE mDescTableRanges[1];

  //ComPtr<ID3D12Resource> mConstBufferUploadHeap;
  //ConstantBuffer mCb;
  //UINT8* mCbAddr;
  //int mAlignSize;
  unique_ptr<BaseUploadHeap<ConstantBuffer>> mConstBuffer;

  void InitConstBuffer();

#pragma endregion
};
