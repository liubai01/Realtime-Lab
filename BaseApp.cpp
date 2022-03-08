#include "BaseApp.h"
#include "DebugOut.h"
#include <D3DCompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>


// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;



LRESULT CALLBACK BaseApp::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (msg == WM_DESTROY)
  {
    // Inform mainloop to exit
    PostQuitMessage(0);
    return 0;
  }

  // Default fallback
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

BaseApp::BaseApp(HINSTANCE hInstance)
{
#if defined(DEBUG) || defined(_DEBUG) 
  // Enable the D3D12 debug layer.
  {
    ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
  }
#endif
  // Initialize the objects required for dumping in Shader, Geometry, etc.
  InitWindow(hInstance);
  InitDevice();
  InitCommandQueue();
  InitFence();
  InitSwapChain();
  InitView();
}

void BaseApp::Init(HINSTANCE hInstance)
{
  // Invoke after dumping in Shader, Geometry, etc.
  InitRTV();
  InitRootSig();
  InitInputLayout();
  InitPSO();
}

BaseApp::~BaseApp()
{
  CloseHandle(mFenceEvent);
}

void BaseApp::Run()
{
  // Main loop

  // A loop at main process is required to keep window open.
  // For ending the process, the loop should halt if user exits.
  // That information is transferred by Win32 message system.
  MSG msg;
  ZeroMemory(&msg, sizeof(MSG));

  while (mIsRunning)
  {
    // Checks the thread message queue for a posted message and retrieves the message (if any exist)
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      // Respond to message by WndProc
      if (msg.message == WM_QUIT)
        break;

      // If message could not be handled by main process, then transfer to WinProc
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    Render();
  }
}

void BaseApp::Render()
{
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
  //mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  //// Specify the buffers we are going to render to.
  D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
  mCommandList->OMSetRenderTargets(1, &rtv, false, nullptr);

  //ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
  //mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  //mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

  for (auto& vbView : mVBuffViews)
  {
    mCommandList->IASetVertexBuffers(0, 1, &vbView);
    //mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //// The draw call here
    //mCommandList->DrawIndexedInstanced(
    //  mBoxGeo->DrawArgs["box"].IndexCount,
    //  1, 0, 0, 0
    //);
    mCommandList->DrawInstanced(3, 1, 0, 0);
  }

  

  // Indicate a state transition on the resource usage.
  trans = CD3DX12_RESOURCE_BARRIER::Transition(
    mRenderTargets[mFrameIdx].Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET, 
    D3D12_RESOURCE_STATE_PRESENT
  );
  mCommandList->ResourceBarrier(1, &trans);

  // Done recording commands.
  HRESULT hr = mCommandList->Close();
  if (FAILED(hr))
  {
    mCommandList->Close();
  }

  // Add the command list to the queue for execution.
  ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
  mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

  Flush();
}

void BaseApp::ResetCommandList()
{
  // Reuse the memory associated with command recording.
  // We can only reset when the associated command lists have finished execution on the GPU.
  HRESULT hr = mCommandAlloc->Reset();
  if (FAILED(hr))
  {
    mIsRunning = false;
    dout::printf("Command Allocated Failed!");
  }

  // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
  // Reusing the command list reuses memory.
  hr = mCommandList->Reset(mCommandAlloc.Get(), mPSO.Get());
  if (FAILED(hr))
  {
    mIsRunning = false;
    dout::printf("Command List Failed!");
  }
}

void BaseApp::InitView()
{
  // Fill out the Viewport
  mViewport.TopLeftX = 0;
  mViewport.TopLeftY = 0;
  mViewport.Width = static_cast<float>(mWidth);
  mViewport.Height = static_cast<float>(mHeight);
  mViewport.MinDepth = 0.0f;
  mViewport.MaxDepth = 1.0f;

  // Fill out a scissor rect
  mScissorRect.left = 0;
  mScissorRect.top = 0;
  mScissorRect.right = mWidth;
  mScissorRect.bottom = mHeight;
}

void BaseApp::InitPSO()
{
  // create a pipeline state object (PSO)

  // In a real application, you will have many pso's. for each different shader
  // or different combinations of shaders, different blend states or different rasterizer states,
  // different topology types (point, line, triangle, patch), or a different number
  // of render targets you will need a pso

  // VS is the only required shader for a pso. You might be wondering when a case would be where
  // you only set the VS. It's possible that you have a pso that only outputs data with the stream
  // output, and not on a render target, which means you would not need anything after the stream
  // output.

  DXGI_SAMPLE_DESC sampleDesc = {};
  sampleDesc.Count = 1;


  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};   // a structure to define a pso
  psoDesc.InputLayout = mInputLayoutDesc;            // the structure describing our input layout
  psoDesc.pRootSignature = mRootSig.Get();           // the root signature that describes the input data this pso needs
  psoDesc.VS = mShader.VertexShaderByteCode();               // structure describing where to find the vertex shader bytecode and how large it is
  psoDesc.PS = mShader.PixelShaderByteCode();                // same as VS but for pixel shader
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
  psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
  psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
  psoDesc.NumRenderTargets = 1; // we are only binding one render target

  // create the PSO
  HRESULT hr = mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(mPSO.GetAddressOf()));
  
  ThrowIfFailed(hr);

}

void BaseApp::InitInputLayout()
{
  // fill out an input layout description structure
  mInputLayoutDesc = {};

  // we can get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
  mInputLayoutDesc.NumElements = mInputLayout.size();
  mInputLayoutDesc.pInputElementDescs = mInputLayout.data();
}

void BaseApp::Flush()
{
  ++mExpectedFenceValue[mFrameIdx];
  HRESULT hr = mCommandQueue->Signal(mFence[mFrameIdx].Get(), mExpectedFenceValue[mFrameIdx]);

  if (FAILED(hr))
  {
    mIsRunning = false;
    return;
  }

  UINT64 nowFenceValue = mFence[mFrameIdx]->GetCompletedValue();
  if (nowFenceValue < mExpectedFenceValue[mFrameIdx])
  {
    hr = mFence[mFrameIdx]->SetEventOnCompletion(mExpectedFenceValue[mFrameIdx], mFenceEvent);
    if (FAILED(hr))
    {
      mIsRunning = false;
      return;
    }

    WaitForSingleObject(mFenceEvent, INFINITE);
  }

  // present the current backbuffer
  hr = mSwapChain->Present(0, 0);
  if (FAILED(hr))
  {
    mIsRunning = false;
    return;
  }

  // frameIndex points to next backBuffer to be flipped to front-end
  mFrameIdx = (mFrameIdx + 1) % mFrameCnt;

}

void BaseApp::InitRTV()
{
  // Render target views
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors = mFrameCnt;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  HRESULT hr = mDevice->CreateDescriptorHeap(
    &rtvHeapDesc,
    IID_PPV_ARGS(mRtvDescriptorHeap.GetAddressOf())
  );

  int rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

  for (int i = 0; i < mFrameCnt; ++i)
  {
    mRenderTargets.emplace_back();
    hr = mSwapChain->GetBuffer(
      i, IID_PPV_ARGS(mRenderTargets[i].GetAddressOf())
    );

    mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, rtvDescriptorSize);
  }
}

D3D12_CPU_DESCRIPTOR_HANDLE BaseApp::CurrentBackBufferView()const
{
  int rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  return CD3DX12_CPU_DESCRIPTOR_HANDLE(
    mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
    mFrameIdx,
    rtvDescriptorSize
  );
}

void BaseApp::InitCommandQueue()
{
  // Initialize three things: CommandQueue, CommandList, CommandAllocator

  // mCommandQueue = CreateCommandQueue(DIRECT)
  D3D12_COMMAND_QUEUE_DESC cqDesc = {};
  cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  HRESULT hr = mDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(mCommandQueue.GetAddressOf()));
  if (FAILED(hr))
  {
    dout::printf("Command Queue Creation Failed!");
  }

  // mCommandAlloc = CreateCommandAllocator(DIERCT)
  hr = mDevice->CreateCommandAllocator(
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    IID_PPV_ARGS(mCommandAlloc.GetAddressOf())
  );
  if (FAILED(hr))
  {
    dout::printf("Command Allocator Failed!");
  }

  // mCommandList = CreateCommandList(DIRECT)
  hr = mDevice->CreateCommandList(
    0,
    D3D12_COMMAND_LIST_TYPE_DIRECT,
    mCommandAlloc.Get(),
    nullptr,
    IID_PPV_ARGS(mCommandList.GetAddressOf())
  );
  mCommandList->Close();
}

void BaseApp::InitWindow(HINSTANCE hInstance)
{
  // Define and register the window class (a variable)
  WNDCLASS wc;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hIcon = LoadIcon(0, IDI_APPLICATION);
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
  wc.lpszMenuName = 0;
  wc.lpszClassName = L"MainWnd";

  RegisterClass(&wc);

  // Handle to the window
  mHwnd = CreateWindowEx(
    NULL,
    L"MainWnd", // long pointer class name
    L"Demo", // long pointer window name
    WS_OVERLAPPEDWINDOW, // dw style
    CW_USEDEFAULT, CW_USEDEFAULT, // x y
    mWidth, mHeight, // width height
    NULL, // window parent
    NULL, // menu
    hInstance, // hInstance
    NULL // lpParam
  );

  ShowWindow(mHwnd, SW_SHOW);
  UpdateWindow(mHwnd);
}

void BaseApp::InitDevice()
{
  HRESULT hr = D3D12CreateDevice(
    nullptr,
    D3D_FEATURE_LEVEL_11_0,
    IID_PPV_ARGS(mDevice.GetAddressOf())
  );

  if (FAILED(hr))
  {
    dout::printf("Device Created Failed!");
  }
}

void BaseApp::InitFence()
{
  mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

  // Create fence for sync. between device(GPU) and host(CPU)
  for (int i = 0; i < mFrameCnt; ++i)
  {
    mFence.push_back(Microsoft::WRL::ComPtr<ID3D12Fence>());

    HRESULT hr = mDevice->CreateFence(
      0,  // initial value
      D3D12_FENCE_FLAG_NONE,
      IID_PPV_ARGS(mFence[i].GetAddressOf())
    );

    mExpectedFenceValue.push_back(0);
  }
}

void BaseApp::InitSwapChain()
{
  // DXGI Factory
  HRESULT hr = CreateDXGIFactory1(
    IID_PPV_ARGS(mDxgiFactory.GetAddressOf())
  );

  // Swap Chain
  DXGI_MODE_DESC backBufferDesc = {};
  backBufferDesc.Width = mWidth;
  backBufferDesc.Height = mHeight;
  backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  DXGI_SAMPLE_DESC sampleDesc = {};
  sampleDesc.Count = 1;

  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  swapChainDesc.BufferCount = mFrameCnt;
  swapChainDesc.BufferDesc = backBufferDesc;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.OutputWindow = mHwnd;
  swapChainDesc.SampleDesc = sampleDesc;
  swapChainDesc.Windowed = true;

  ComPtr<IDXGISwapChain> tmpSwapChain;
  mDxgiFactory->CreateSwapChain(
    mCommandQueue.Get(),
    &swapChainDesc,
    reinterpret_cast<IDXGISwapChain**>(mSwapChain.GetAddressOf())
  );
  tmpSwapChain = nullptr; // To avoid the resources bound to swapChain be released
}

void BaseApp::InitRootSig()
{
  CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
  rootSigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> signature;
  HRESULT hr = D3D12SerializeRootSignature(
    &rootSigDesc,
    D3D_ROOT_SIGNATURE_VERSION_1,
    signature.GetAddressOf(),
    nullptr
  );
  if (FAILED(hr))
  {
    dout::printf("Signature Blob(CPU) Failed");
  }

  hr = mDevice->CreateRootSignature(
    0, 
    signature->GetBufferPointer(), 
    signature->GetBufferSize(), 
    IID_PPV_ARGS(mRootSig.GetAddressOf())
  );
  if (FAILED(hr))
  {
    dout::printf("RootSignature Created Failed!");
  }

}

