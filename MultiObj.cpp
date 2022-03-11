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

#include "MultiObj.h"

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

  delete app;

  return 0;
}

MyApp::MyApp(HINSTANCE hInstance) : BaseApp(hInstance) {
  BaseGeometry<Vertex> cube;

  // Set data
  XMFLOAT4 color = { 0.8f, 0.2f, 0.2f, 1.0f };
  cube.mName = "Cube Red";
  SetCubeGeo(cube, color);

  BaseRenderingObj* obj = RegisterGeo(cube);
  obj->SetPos(2.0f, 4.0f, 0.0f);
  obj->SetRot(XM_PI / 4.0f, 0.0f, 0.0f);

  cube.mName = "Cube Green";
  color = { 0.2f, 0.4f, 0.2f, 1.0f };
  SetCubeGeo(cube, color);

  obj = RegisterGeo(cube);
  obj->SetPos(0.0f, 0.0f, 0.0f);
  obj->SetScale(6.0f, 0.5f, 6.0f);

  cube.mName = "Cube Blue";
  color = { 0.2f, 0.2f, 0.8f, 1.0f };
  SetCubeGeo(cube, color);

  obj = RegisterGeo(cube);
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

  InitConstBuffer();

  Init(hInstance);
}


void MyApp::Render() {
  ResetCommandList();

  mCommandList->SetGraphicsRootSignature(mRootSig.Get());
  mCommandList->RSSetViewports(1, &mViewport);
  mCommandList->RSSetScissorRects(1, &mScissorRect);

  //// Indicate a state transition on the resource usage.
  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_PRESENT,
    D3D12_RESOURCE_STATE_RENDER_TARGET
  );
  mCommandList->ResourceBarrier(
    1,
    &trans
  );

  // Clear the back buffer and depth buffer.
  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  mCommandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
  mCommandList->ClearDepthStencilView(DepthBufferView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  //// Specify the buffers we are going to render to.
  D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthBufferView();
  mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

  ID3D12DescriptorHeap* descriptorHeaps[] = { mConstDescHeap.Get() };
  mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mConstDescHeap->GetGPUDescriptorHandleForHeapStart());
  for (auto& obj : mObjs)
  {
    D3D12_INDEX_BUFFER_VIEW ibView = obj.mIndexBufferView;
    D3D12_VERTEX_BUFFER_VIEW vbView = obj.mVertexBufferView;

    mCommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->IASetVertexBuffers(0, 1, &vbView);
    mCommandList->IASetIndexBuffer(&ibView);

    mCommandList->DrawIndexedInstanced(
      obj.mNumIndex,
      1, 0, 0, 0
    );

    cbvHandle.Offset(1, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  }

  // Indicate a state transition on the resource usage.
  trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT
  );
  mCommandList->ResourceBarrier(1, &trans);

  Flush();
  Swap();
}

void MyApp::Update() {
  XMFLOAT4X4 mView = Identity4x4();

  static float mTheta = 1.5f * XM_PI;
  float mPhi = XM_PIDIV4;
  float mRadius = 20.0f;

  mTheta += XM_PI * mTimeDelta * 0.1f;

  // Convert Spherical to Cartesian coordinates.
  float x = mRadius * sinf(mPhi) * cosf(mTheta);
  float z = mRadius * sinf(mPhi) * sinf(mTheta);
  float y = mRadius * cosf(mPhi);


  XMVECTOR view_dir = XMVectorSet(x, y, z, 0);
  XMVECTOR light_dir = XMVectorSet(1.0f, 1.0f, 1.0f, 0);
  XMStoreFloat4(&mConstBuffer->mData.LightDir, XMVector3Normalize(light_dir));

  // Build the view matrix.
  //XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
  XMVECTOR pos = view_dir;
  XMVECTOR target = XMVectorZero();
  XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

  XMStoreFloat4(&mConstBuffer->mData.EyePos, pos);

  XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
  XMStoreFloat4x4(&mView, view);

  XMMATRIX proj = XMLoadFloat4x4(&mProj);

  mObjs[2].mPosition.z = x / 2.0f;

  // copy our ConstantBuffer instance to the mapped constant buffer resource
  for (size_t i = 0; i < mObjs.size(); ++i)
  {
    XMMATRIX worldViewProj = mObjs[i].GetWorldMatrix() * view * proj;

    // Update the constant buffer with the latest worldViewProj matrix.
    XMStoreFloat4x4(&mConstBuffer->mData.WorldViewProj, XMMatrixTranspose(worldViewProj));
    XMStoreFloat4x4(&mConstBuffer->mData.World, XMMatrixTranspose(mObjs[i].GetWorldMatrix()));
    XMStoreFloat4x4(&mConstBuffer->mData.RSInvT, XMMatrixTranspose(mObjs[i].GetRSInvT()));

    mConstBuffer->Upload(i);
  }
}

void MyApp::InitConstBuffer()
{
  // create a descriptor range (descriptor table) and fill it out
  // this is a range of descriptors inside a descriptor heap
  mDescTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
  mDescTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
  mDescTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
  mDescTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
  mDescTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

  // create a descriptor table
  D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
  descriptorTable.NumDescriptorRanges = _countof(mDescTableRanges); // we only have one range
  descriptorTable.pDescriptorRanges = &mDescTableRanges[0]; // the pointer to the beginning of our ranges array


  // create a root parameter and fill it out
  mRootParams.emplace_back();
  mRootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
  mRootParams[0].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
  mRootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = mObjs.size();
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  HRESULT hr = mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mConstDescHeap.GetAddressOf()));
  if (FAILED(hr))
  {
    mIsRunning = false;
    return;
  }

  mConstBuffer = make_unique<BaseUploadHeap<ConstantBuffer>>(mObjs.size(), mDevice.Get());
  mConstBuffer->ConstructDescHeap(mDevice.Get(), mConstDescHeap.Get());
}