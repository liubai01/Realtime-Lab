/*
* MIT License

Copyright (c) 2021 Yintao, Xu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// This version of code omits many error handling for simplicity.
// Add them back in real projects.

#include <windows.h>
#include "DebugOut.h"
#include "BaseApp.h"
#include "BaseGeometry.h"

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
  vector<BaseGeometry<Vertex>> mGeos;

  MyApp(HINSTANCE hInstance) : BaseApp(hInstance) {
    // Set data
    mGeos.emplace_back();
    mGeos[0].mName = "Cube Red";
    XMFLOAT4 color = { 0.8f, 0.2f, 0.2f, 1.0f };
    SetCubeGeo(mGeos[0], color);
    
    BaseRenderingObj* obj = RegisterGeo(mGeos[0]);
    obj->SetPos(2.0f, 4.0f, 0.0f);
    obj->SetRot(XM_PI / 4.0f, 0.0f, 0.0f);

    mGeos.emplace_back();
    mGeos[1].mName = "Cube Green";
    color = { 0.2f, 0.4f, 0.2f, 1.0f };
    SetCubeGeo(mGeos[1], color);

    obj = RegisterGeo(mGeos[1]);
    obj->SetPos(0.0f, 0.0f, 0.0f);
    obj->SetScale(6.0f, 0.5f, 6.0f);

    mGeos.emplace_back();
    mGeos[2].mName = "Cube Blue";
    color = { 0.2f, 0.2f, 0.8f, 1.0f };
    SetCubeGeo(mGeos[2], color);

    obj = RegisterGeo(mGeos[2]);
    obj->SetPos(-2.0f, 4.0f, 4.0f);
    obj->SetRot(-XM_PI / 4.0f, XM_PI / 8.0f, 0.0f);


    // Set input layout
    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    mShader.AddVertexShader("VertexShader.hlsl");
    mShader.AddPixelShader("PixelShader.hlsl");

    Init(hInstance);
  }
};

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
  BaseApp* app = new MyApp(hInstance);

  try
  {
    app->Run();
  }
  catch (dout::DxException e) {
    dout::printf("%s\n", e.ToString().c_str());
  }
  

  return 0;
}