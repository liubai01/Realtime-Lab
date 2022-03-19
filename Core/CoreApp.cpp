#include "CoreApp.h"


CoreApp::CoreApp(HINSTANCE hInstance) : BaseApp(hInstance)
{
  mDrawContext = make_unique<BaseDrawContext>(mDevice.Get());

  //// Set input layout
  //mDrawContext->mInputLayout =
  //{
  //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  //    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  //    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  //};

  //mDrawContext->mShader.AddVertexShader("VertexShader.hlsl");
  //mDrawContext->mShader.AddPixelShader("PixelShader.hlsl");
}

void CoreApp::Update()
{

}

void CoreApp::Render()
{
  mDrawContext->ResetCommandList();
  ID3D12GraphicsCommandList* commandList = mDrawContext->mCommandList.Get();

  commandList->SetGraphicsRootSignature(mDrawContext->mRootSig.Get());
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
  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  commandList->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);

  // Specify the buffers we are going to render to.
  D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
  commandList->OMSetRenderTargets(1, &rtv, false, nullptr);

  trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT
  );
  commandList->ResourceBarrier(
    1,
    &trans
  );

  // Not flush due to UI
  Flush(mDrawContext->mCommandList.Get());
  Swap();
}