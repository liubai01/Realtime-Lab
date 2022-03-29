#include "CoreApp.h"

#include "../Core/Component/CoreMeshComponent.h"
#include "CoreGeometryUtils.h"

CoreApp::CoreApp(HINSTANCE hInstance) : BaseApp(hInstance)
{
  mDrawContext = make_unique<BaseDrawContext>(mDevice.Get());
  mUploadCmdList = make_unique<BaseDirectCommandList>(mDevice.Get());

  // Set input layout
  mDrawContext->mInputLayout =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  mDrawContext->mShader.AddVertexShader("Core\\Shader\\ShadowMapVertexShader.hlsl");
  mDrawContext->mShader.AddPixelShader("Core\\Shader\\ShadowMapPixelShader.hlsl");
}

void CoreApp::Start()
{
  shared_ptr<BaseObject> cubeObj = CreateObject("Cube Red");
  shared_ptr<CoreGeometry> cubeGeo = make_shared<CoreGeometry>(GetCubeGeometry());
  shared_ptr<CoreMeshComponent> meshComponent = make_shared<CoreMeshComponent>(cubeGeo);
  cubeObj->AddComponent(meshComponent);

}

void CoreApp::Update()
{

}

void CoreApp::Render()
{
  mRuntimeHeap->Reset();

  // 1. Upload geoemetry(vertices, indices) if it has not been uploaded before
  // 2. Register constant buffer
  mUploadCmdList->ResetCommandList();
  for (auto& elem : *mObjs)
  {
    shared_ptr<BaseObject>& obj = elem.second;
    obj->mTransform.RegisterRuntimeHandle(mRuntimeHeap);

    for (auto component : obj->mComponents)
    {
      if (component->mComponentType == BaseComponentType::BASE_COMPONENT_MESH)
      {
        CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));
        if (!com->mUploaded)
        {
          com->Upload(mDevice.Get(), mUploadCmdList->mCommandList.Get());
        }
      }
    }
  }
  Enqueue(mUploadCmdList->mCommandList.Get());

  mDrawContext->ResetCommandList();
  ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

  commandList->SetGraphicsRootSignature(mDrawContext->GetRootSig());
  commandList->RSSetViewports(1, &mMainCamera->mViewport);
  commandList->RSSetScissorRects(1, &mMainCamera->mScissorRect);

  auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_PRESENT,
    D3D12_RESOURCE_STATE_RENDER_TARGET
  );
  commandList->ResourceBarrier(
    1,
    &trans
  );

  // Clear the back buffer and depth buffer.
  D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthBufferView();

  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
  commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
  for (auto& elem : *mObjs)
  {
    shared_ptr<BaseObject>& obj = elem.second;
    for (auto component : obj->mComponents)
    {
      if (component->mComponentType == BaseComponentType::BASE_COMPONENT_MESH)
      {
        CoreMeshComponent* com = static_cast<CoreMeshComponent*>(&(*component));

        D3D12_INDEX_BUFFER_VIEW ibView = com->mIndexBufferView;
        D3D12_VERTEX_BUFFER_VIEW vbView = com->mVertexBufferView;

        commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &vbView);
        commandList->IASetIndexBuffer(&ibView);

        commandList->DrawIndexedInstanced(
          com->mGeo->mIndices.size(),
          1, 0, 0, 0
        );
      }
    }
  }
  

  trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT
  );
  commandList->ResourceBarrier(
    1,
    &trans
  );
  Flush(mDrawContext->mCommandList.Get());
  Swap();
}