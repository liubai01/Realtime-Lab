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

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "ThirdParty/tiny_obj_loader.h"



MyApp::MyApp(HINSTANCE hInstance) : BaseApp(hInstance) {
  mShadowMapCamera = make_unique<BaseCamera>(1024.0f * 2.0f, 768.0f * 2.0f);
  mDrawContext = make_unique<BaseDrawContext>(mDevice.Get());

  // Set input layout
  mDrawContext->mInputLayout =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  mDrawContext->mShader.AddVertexShader("VertexShader.hlsl");
  mDrawContext->mShader.AddPixelShader("PixelShader.hlsl");

  // load image
  mTexture = make_unique<BaseImage>();
  mTexture->Read("Asset\\Water.jpg");

  mDrawContext->ResetCommandList();
  mTexture->Upload(mDevice.Get(), mDrawContext->mCommandList.Get());

  BaseGeometry<Vertex> cube;

  // Set data
  cube.mName = "Cube Red";
  SetCubeGeo(cube);

  BaseRenderingObj* obj = RegisterGeo(cube, mDrawContext);
  obj->SetPos(4.0f, 4.0f, 0.0f);
  obj->SetRot(XM_PI / 4.0f, 0.0f, 0.0f);

  cube.mName = "Base";
  SetCubeGeo(cube);

  obj = RegisterGeo(cube, mDrawContext);
  obj->SetPos(0.0f, 0.0f, 0.0f);
  obj->SetScale(6.0f, 0.5f, 6.0f);

  cube.mName = "Cube Blue";
  SetCubeGeo(cube);

  obj = RegisterGeo(cube, mDrawContext);
  obj->SetPos(-2.0f, 4.0f, -4.0f);
  obj->SetRot(-XM_PI / 4.0f, XM_PI / 8.0f, 0.0f);

  LoadObj();

  Flush(mDrawContext->mCommandList.Get());

  InitShadowDepth();

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = mObjs.size() * 2 + 2; // three global constant buffer + one texture
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  HRESULT hr = mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mDescHeap.GetAddressOf()));
  ThrowIfFailed(hr);

  InitUI();
  InitConstBuffer();
  InitMatBuffer();
  InitTextureBuffer();

  mDrawContext->Init();
}

MyApp::~MyApp()
{
  ImGui_ImplDX12_Shutdown();
}

void MyApp::LoadObj()
{
  BaseGeometry<Vertex> bunny;

  std::string inputfile = "Asset\\bunny.obj";
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = "./"; // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(inputfile, reader_config)) {
    if (!reader.Error().empty()) {
      dout::printf("TinyObjReader: %s\n", reader.Error().c_str());
    }
    exit(1);
  }

  if (!reader.Warning().empty()) {
    dout::printf("TinyObjReader: %s\n", reader.Warning().c_str());
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  auto& materials = reader.GetMaterials();

  assert(shapes.size() == 1);

  // Loop over vertices
  Vertex vout = {};
  for (size_t v = 0; v < attrib.vertices.size(); v+=3) {

    tinyobj::real_t vx = attrib.vertices[v + 0];
    tinyobj::real_t vy = attrib.vertices[v + 1];
    tinyobj::real_t vz = attrib.vertices[v + 2];
    vout.pos = {vx, vy, vz};


    tinyobj::real_t nx = attrib.normals[v + 0];
    tinyobj::real_t ny = attrib.normals[v + 1];
    tinyobj::real_t nz = attrib.normals[v + 2];
    vout.normal = { nx, ny, nz };
    
    bunny.mVertices.push_back(vout);
  }

  // Loop over faces
  size_t index_offset = 0;

  for (size_t f = 0; f < shapes[0].mesh.num_face_vertices.size(); f++) {
    size_t fv = size_t(shapes[0].mesh.num_face_vertices[f]);
    assert(fv == 3);

    bunny.mIndices.push_back(
      shapes[0].mesh.indices[index_offset + 0].vertex_index
    );
    bunny.mIndices.push_back(
      shapes[0].mesh.indices[index_offset + 1].vertex_index
    );
    bunny.mIndices.push_back(
      shapes[0].mesh.indices[index_offset + 2].vertex_index
    );

    index_offset += fv;

  }
  //dout::printf("Index num: %d\n", shapes[0].mesh.num_face_vertices.size());

  bunny.mName = "Bunny";
  BaseRenderingObj* obj = RegisterGeo(bunny, mDrawContext);
  obj->SetScale(4.0f, 4.0f, 4.0f);
}

void MyApp::Start() {
  // copy our ConstantBuffer instance to the mapped constant buffer resource
  mMatBuffer->mData = BaseMaterialConsts();
  for (size_t i = 0; i < mObjs.size(); ++i)
  {
    if (i == 0) {
      mMatBuffer->mData.Kd = {0.8f, 0.0f, 0.0f, 1.0f};
    }
    else if (i == 2) {
      mMatBuffer->mData.Kd = { 0.0f, 0.0f, 0.8f, 1.0f };
    }
    else if (i == 3) {
      mMatBuffer->mData.Kd = { 0.5f, 0.5f, 0.5f, 1.0f };
    }

    if (i == 1) {
      mMatBuffer->mData.IsTextured = true;
    }
    else {
      mMatBuffer->mData.IsTextured = false;
    }
    mMatBuffer->Upload(i);
  }
}

void MyApp::InitUI()
{
  D3D12_DESCRIPTOR_HEAP_DESC  heapDesc = {};
  heapDesc.NumDescriptors = 1; // the UI heap only need one descriptor
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  HRESULT hr = mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mUIDescHeap.GetAddressOf()));
  ThrowIfFailed(hr);

  mClearColor = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);

  ImGui_ImplDX12_Init(mDevice.Get(), mFrameCnt,
    DXGI_FORMAT_R8G8B8A8_UNORM, mUIDescHeap.Get(),
    mUIDescHeap->GetCPUDescriptorHandleForHeapStart(),
    mUIDescHeap->GetGPUDescriptorHandleForHeapStart());
}

void MyApp::Render() {
  RenderShadowMap();
  RenderObjects();
  RenderUI();
  Swap();
}

void MyApp::RenderUI() {
  ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();
  HRESULT hr = mDrawContext->mCommandAlloc->Reset();
  commandList->Reset(mDrawContext->mCommandAlloc.Get(), nullptr);

  ImGui::Render();


  D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
  commandList->OMSetRenderTargets(1, &rtv, false, nullptr);
  commandList->SetDescriptorHeaps(1, mUIDescHeap.GetAddressOf());
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT
  );
  commandList->ResourceBarrier(1, &trans);

  Flush(mDrawContext->mCommandList.Get());
}

D3D12_CPU_DESCRIPTOR_HANDLE MyApp::ShadowDepthBufferView() const
{
  CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mShadowDsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
  return dsvHandle;
}

void MyApp::InitShadowDepth()
{
  // Create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
  dsvHeapDesc.NumDescriptors = 1;
  dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  HRESULT hr = mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mShadowDsDescriptorHeap.GetAddressOf()));
  if (FAILED(hr))
  {
    mIsRunning = false;
    return;
  }

  D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
  depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
  depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

  D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
  depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
  depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
  depthOptimizedClearValue.DepthStencil.Stencil = 0;

  auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  auto rdesc = CD3DX12_RESOURCE_DESC::Tex2D(
    DXGI_FORMAT_D32_FLOAT, 
    static_cast<UINT64>(mShadowMapCamera->mWidth), 
    static_cast<UINT>(mShadowMapCamera->mHeight),
    1, 0, 1, 0, 
    D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
  );
  mDevice->CreateCommittedResource(
    &hprop,
    D3D12_HEAP_FLAG_NONE,
    &rdesc,
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &depthOptimizedClearValue,
    IID_PPV_ARGS(mShadowDepthStencilBuffer.GetAddressOf())
  );
  mShadowDsDescriptorHeap->SetName(L"Shadow Depth/Stencil Resource Heap");

  mDevice->CreateDepthStencilView(
    mShadowDepthStencilBuffer.Get(),
    &depthStencilDesc,
    mShadowDsDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
  );
}

void MyApp::RenderShadowMap()
{
  D3D12_VIEWPORT viewport = mShadowMapCamera->mViewport; // area that output from rasterizer will be stretched to.
  D3D12_RECT scissorRect = mShadowMapCamera->mScissorRect; // the area to draw in. pixels outside that area will not be drawn onto
  XMMATRIX proj = XMLoadFloat4x4(&mShadowMapCamera->mProj);
  MatrixBuild(mSMPos, proj);

  mDrawContext->ResetCommandList();
  
  ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();
  mCommandQueue->BeginEvent(1, "ShadowMap", sizeof("ShadowMap"));
  commandList->SetGraphicsRootSignature(mDrawContext->mRootSig.Get());
  commandList->RSSetViewports(1, &viewport);
  commandList->RSSetScissorRects(1, &scissorRect);

  D3D12_CPU_DESCRIPTOR_HANDLE dsv = ShadowDepthBufferView();

  // Clear the back buffer and depth buffer.
  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  // Specify the buffers we are going to render to.

  commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

  ID3D12DescriptorHeap* descriptorHeaps[] = { mDescHeap.Get() };
  commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  // texture handle to 2 (root param [2])
  auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescHeap->GetGPUDescriptorHandleForHeapStart());
  handle.Offset(mObjs.size() * 2, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  commandList->SetGraphicsRootDescriptorTable(2, handle);

  handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescHeap->GetGPUDescriptorHandleForHeapStart());
  for (auto& obj : mObjs)
  {
    auto handleMat = CD3DX12_GPU_DESCRIPTOR_HANDLE(handle);
    handleMat.Offset(mObjs.size(), mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    D3D12_INDEX_BUFFER_VIEW ibView = obj.mIndexBufferView;
    D3D12_VERTEX_BUFFER_VIEW vbView = obj.mVertexBufferView;

    // constant buffer handle to 0 (root param [0])
    commandList->SetGraphicsRootDescriptorTable(0, handle);
    commandList->SetGraphicsRootDescriptorTable(1, handleMat);

    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vbView);
    commandList->IASetIndexBuffer(&ibView);

    commandList->DrawIndexedInstanced(
      obj.mNumIndex,
      1, 0, 0, 0
    );

    handle.Offset(1, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  };

  Flush(mDrawContext->mCommandList.Get());

  mCommandQueue->EndEvent();
}

void MyApp::RenderObjects()
{
  XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, static_cast<float>(mWidth) / mHeight, 1.0f, 1000.0f);
  MatrixBuild(mViewPos, proj);

  mDrawContext->ResetCommandList();
  ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

  commandList->SetGraphicsRootSignature(mDrawContext->mRootSig.Get());
  commandList->RSSetViewports(1, &mMainCamera->mViewport);
  commandList->RSSetScissorRects(1, &mMainCamera->mScissorRect);

  //// Indicate a state transition on the resource usage.
  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_PRESENT,
    D3D12_RESOURCE_STATE_RENDER_TARGET
  );
  commandList->ResourceBarrier(
    1,
    &trans
  );

  trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mShadowDepthStencilBuffer.Get(),
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
  );
  commandList->ResourceBarrier(
    1,
    &trans
  );

  // Clear the back buffer and depth buffer.
  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  commandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);
  commandList->ClearDepthStencilView(DepthBufferView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  //// Specify the buffers we are going to render to.
  D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthBufferView();
  commandList->OMSetRenderTargets(1, &rtv, false, &dsv);

  ID3D12DescriptorHeap* descriptorHeaps[] = { mDescHeap.Get() };
  commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  // texture handle to 2 (root param [2])
  auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescHeap->GetGPUDescriptorHandleForHeapStart());
  handle.Offset(mObjs.size() * 2, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  commandList->SetGraphicsRootDescriptorTable(2, handle);

  handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mDescHeap->GetGPUDescriptorHandleForHeapStart());
  for (auto& obj : mObjs)
  {
    auto handleMat = CD3DX12_GPU_DESCRIPTOR_HANDLE(handle);
    handleMat.Offset(mObjs.size(), mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    D3D12_INDEX_BUFFER_VIEW ibView = obj.mIndexBufferView;
    D3D12_VERTEX_BUFFER_VIEW vbView = obj.mVertexBufferView;

    // constant buffer handle to 0 (root param [0])
    commandList->SetGraphicsRootDescriptorTable(0, handle);
    commandList->SetGraphicsRootDescriptorTable(1, handleMat);

    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vbView);
    commandList->IASetIndexBuffer(&ibView);

    commandList->DrawIndexedInstanced(
      obj.mNumIndex,
      1, 0, 0, 0
    );

    handle.Offset(1, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  }

  // Indicate a state transition on the resource usage.
  //trans = CD3DX12_RESOURCE_BARRIER::Transition(
  //  mRenderTargets[mFrameIdx].Get(),
  //  D3D12_RESOURCE_STATE_RENDER_TARGET,
  //  D3D12_RESOURCE_STATE_PRESENT
  //);
  //commandList->ResourceBarrier(1, &trans);
  // not transform back now because we still would like to reneder UI

  trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mShadowDepthStencilBuffer.Get(),
    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
    D3D12_RESOURCE_STATE_DEPTH_WRITE 
  );
  commandList->ResourceBarrier(
    1,
    &trans
  );

  // Not flush due to UI
  Flush(mDrawContext->mCommandList.Get());
}

void MyApp::MatrixBuild(XMVECTOR& viewPos, XMMATRIX& proj)
{
  // Build the view matrix.
  XMVECTOR target = XMVectorZero();
  XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

  XMStoreFloat4(&mConstBuffer->mData.EyePos, viewPos);

  XMMATRIX view = XMMatrixLookAtLH(viewPos, target, up);

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

void MyApp::Update() {
  static float mTheta = 1.5f * XM_PI;
  float mPhi = XM_PIDIV4;
  float mRadius = 20.0f;

  mTheta += XM_PI * mTimeDelta * 0.1f;

  // Convert Spherical to Cartesian coordinates.
  float x = mRadius * sinf(mPhi) * cosf(mTheta);
  float z = mRadius * sinf(mPhi) * sinf(mTheta);
  float y = mRadius * cosf(mPhi);


  mViewPos = XMVectorSet(x, y, z, 0);
  //mLightDir = XMVector3Normalize(XMVectorSet(1.0f, 1.8f, 1.0f, 0.0f));
  mLightDir = XMVector3Normalize(XMVectorSet(-x / mRadius, 1.0f, z / mRadius, 0.0f));
  mSMPos = mLightDir * 25.0f;

  XMStoreFloat4(&mConstBuffer->mData.LightDir, mLightDir);

  XMVECTOR target = XMVectorZero();
  XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  XMMATRIX view = XMMatrixLookAtLH(mSMPos, target, up);
  //XMMATRIX proj = XMMatrixPerspectiveFovLH(
  //  0.25f * 3.1415926535f,
  //  static_cast<float>(mShadowMapCamera->mWidth) / mShadowMapCamera->mHeight, 
  //  1.0f, 
  //  1000.0f
  //);

  XMMATRIX proj = XMLoadFloat4x4(&mShadowMapCamera->mProj);
  XMMATRIX viewProj = view * proj;
  XMStoreFloat4x4(&mConstBuffer->mData.ShadowViewProj, XMMatrixTranspose(viewProj));

  // Start the Dear ImGui frame
  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  static float f = 0.0f;
  static int counter = 0;

  ImGui::Begin("Control Pannel");

  ImGui::Text("Shadow mapping demo");               

  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
  ImGui::ColorEdit3("clear color", (float*)&mClearColor); // Edit 3 floats representing a color
  mMatBuffer->mData.Kd = { mClearColor.x, mClearColor.y, mClearColor.z, mClearColor.w };
  mMatBuffer->Upload(3);

  if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    counter++;
  ImGui::SameLine();
  ImGui::Text("counter = %d", counter);

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  //auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mUIDescHeap->GetGPUDescriptorHandleForHeapStart());
  //handle.Offset(1, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  //ImGui::Image((ImTextureID) handle.ptr, ImVec2( mShadowWidth / 10.0f, mShadowHeight / 10.0f));

  ImGui::End();

}

void MyApp::InitMatBuffer()
{
  // texture desc
  mDrawContext->mDescTableRanges.emplace_back(1);
  vector<D3D12_DESCRIPTOR_RANGE>& descTableRanges= mDrawContext->mDescTableRanges.back();

  descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
  descTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
  descTableRanges[0].BaseShaderRegister = 1; // start index of the shader registers in the range
  descTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
  descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

  // create a descriptor table
  D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
  descriptorTable.NumDescriptorRanges = descTableRanges.size(); // we only have one range
  descriptorTable.pDescriptorRanges = descTableRanges.data(); // the pointer to the beginning of our ranges array

  // create a root parameter and fill it out
  mDrawContext->mRootParams.emplace_back();
  mDrawContext->mRootParams.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
  mDrawContext->mRootParams.back().DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
  mDrawContext->mRootParams.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

  mMatBuffer = make_unique<BaseUploadHeap<BaseMaterialConsts>>(mObjs.size(), mDevice.Get());
  mMatBuffer->AppendDesc(mDevice.Get(), mDescHeap.Get(), mObjs.size());
}

void MyApp::InitConstBuffer()
{
  // global desc
  // create a descriptor range (descriptor table) and fill it out
  // this is a range of descriptors inside a descriptor heap
  mDrawContext->mDescTableRanges.emplace_back(1);
  vector<D3D12_DESCRIPTOR_RANGE>& descTableRanges = mDrawContext->mDescTableRanges.back();

  descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
  descTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
  descTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
  descTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
  descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

  // create a descriptor table
  D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
  descriptorTable.NumDescriptorRanges = descTableRanges.size(); // we only have one range
  descriptorTable.pDescriptorRanges = descTableRanges.data(); // the pointer to the beginning of our ranges array


  // create a root parameter and fill it out
  mDrawContext->mRootParams.emplace_back();
  mDrawContext->mRootParams.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
  mDrawContext->mRootParams.back().DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
  mDrawContext->mRootParams.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

  mConstBuffer = make_unique<BaseUploadHeap<GlobalConsts>>(mObjs.size(), mDevice.Get());
  mConstBuffer->AppendDesc(mDevice.Get(), mDescHeap.Get());


}

void MyApp::InitTextureBuffer()
{
  // create a descriptor range (descriptor table) and fill it out
  // this is a range of descriptors inside a descriptor heap
  mDrawContext->mDescTableRanges.emplace_back(1);
  vector<D3D12_DESCRIPTOR_RANGE>& descTableRanges = mDrawContext->mDescTableRanges.back();

  descTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of shader resource views (descriptors)
  descTableRanges[0].NumDescriptors = 2; // we only have one texture right now, so the range is only 1
  descTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
  descTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
  descTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

  // create a descriptor table
  D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
  descriptorTable.NumDescriptorRanges = descTableRanges.size(); // we only have one range
  descriptorTable.pDescriptorRanges = descTableRanges.data(); // the pointer to the beginning of our ranges array

  mDrawContext->mRootParams.emplace_back();
  mDrawContext->mRootParams.back().ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
  mDrawContext->mRootParams.back().DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
  mDrawContext->mRootParams.back().ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // our pixel shader will be the only shader accessing this parameter for now

  mTexture->AppendDesc(mDevice.Get(), mDescHeap.Get(), mObjs.size() * 2);

  auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDescHeap->GetCPUDescriptorHandleForHeapStart());
  handle.Offset(mObjs.size() * 2 + 1, mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
  // now we create a shader resource view (descriptor that points to the texture and describes it)
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  mDevice->CreateShaderResourceView(mShadowDepthStencilBuffer.Get(), &srvDesc, handle);
}

void SetCubeGeo(BaseGeometry<Vertex>& geo)
{
  geo.mVertices = {
    // front
    { { -1.0f, +1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
    { { +1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
    { { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
    { { +1.0f, +1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },
    // right
    { { +1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
    { { +1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
    { { +1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
    { { +1.0f, +1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },
    // left
    { { -1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
    { { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
    { { -1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
    { { -1.0f, +1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },
    // back
    { { +1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
    { { -1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
    { { +1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
    { { -1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },
    // top
    { { -1.0f, +1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
    { { +1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
    { { +1.0f, +1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
    { { -1.0f, +1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },
    // bottom
    { { +1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f} },
    { { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f} },
    { { +1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f} },
    { { -1.0f, -1.0f, +1.0f }, { 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f} },
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