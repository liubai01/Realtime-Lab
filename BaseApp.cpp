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
  mTimer = clock();

  // Initialize the objects required for dumping in Shader, Geometry, etc.
  InitWindow(hInstance);
  InitDevice();
  InitCommandQueue();
  InitFence();
  InitSwapChain();
  InitView();
  InitDepth();
  InitRTV();
}

void BaseApp::Init(HINSTANCE hInstance)
{
  // Invoke after dumping in Shader, Geometry, etc.
  InitConstBuffer();
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

    mTimeDelta = float(clock() - mTimer) / CLOCKS_PER_SEC;
    mTimer = clock();
    Update();
    Render();
  }
}

void BaseApp::Update()
{
  // update app logic, such as moving the camera or figuring out what objects are in view
  static float rIncrement = 0.02f * mTimeDelta;
  static float gIncrement = 0.04f * mTimeDelta;
  static float bIncrement = 0.08f * mTimeDelta;

  mCbColorMultiplierData.colorMultiplier.x += rIncrement;
  mCbColorMultiplierData.colorMultiplier.y += gIncrement;
  mCbColorMultiplierData.colorMultiplier.z += bIncrement;

  if (mCbColorMultiplierData.colorMultiplier.x >= 1.0 || mCbColorMultiplierData.colorMultiplier.x <= 0.0)
  {
    mCbColorMultiplierData.colorMultiplier.x = mCbColorMultiplierData.colorMultiplier.x >= 1.0f ? 1.0f : 0.0f;
    rIncrement = -rIncrement;
  }
  if (mCbColorMultiplierData.colorMultiplier.y >= 1.0 || mCbColorMultiplierData.colorMultiplier.y <= 0.0)
  {
    mCbColorMultiplierData.colorMultiplier.y = mCbColorMultiplierData.colorMultiplier.y >= 1.0f ? 1.0f : 0.0f;
    gIncrement = -gIncrement;
  }
  if (mCbColorMultiplierData.colorMultiplier.z >= 1.0 || mCbColorMultiplierData.colorMultiplier.z <= 0.0)
  {
    mCbColorMultiplierData.colorMultiplier.z = mCbColorMultiplierData.colorMultiplier.z >= 1.0f ? 1.0f : 0.0f;
    bIncrement = -bIncrement;
  }

  // copy our ConstantBuffer instance to the mapped constant buffer resource
  memcpy(mCbColorMultiplierAddr[mFrameIdx], &mCbColorMultiplierData, sizeof(mCbColorMultiplierData));

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
  mCommandList->ClearDepthStencilView(DepthBufferView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  //// Specify the buffers we are going to render to.
  D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthBufferView();
  mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

  ID3D12DescriptorHeap* descriptorHeaps[] = { mConstDescHeap[mFrameIdx].Get() };
  mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
  mCommandList->SetGraphicsRootDescriptorTable(0, mConstDescHeap[mFrameIdx]->GetGPUDescriptorHandleForHeapStart());


  for (auto& geo: mGeos)
  {
    D3D12_INDEX_BUFFER_VIEW ibView = *geo.mIndexBufferView;
    D3D12_VERTEX_BUFFER_VIEW vbView = *geo.mVertexBufferView;

    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->IASetVertexBuffers(0, 1, &vbView);
    mCommandList->IASetIndexBuffer(&ibView);

    mCommandList->DrawIndexedInstanced(
      geo.mNumIndex,
      1, 0, 0, 0
    );
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

void BaseApp::InitConstBuffer()
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
  mRootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // our pixel shader will be the only shader accessing this parameter for now


  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = 1;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  for (int i = 0; i < mFrameCnt; ++i)
  {
    mConstDescHeap.emplace_back();
    mConstBufferUploadHeap.emplace_back();
    mCbColorMultiplierAddr.emplace_back();
  }

  ZeroMemory(&mCbColorMultiplierData, sizeof(mCbColorMultiplierData));

  for (int i = 0; i < mFrameCnt; ++i)
  {
    
    HRESULT hr = mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mConstDescHeap[i].GetAddressOf()));
    if (FAILED(hr))
    {
      mIsRunning = false;
    }

    
    auto hprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto rdesc = CD3DX12_RESOURCE_DESC::Buffer(1024 * 64);
    hr = mDevice->CreateCommittedResource(
      &hprop, // this heap will be used to upload the constant buffer data
      D3D12_HEAP_FLAG_NONE, // no flags
      &rdesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
      D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
      nullptr, // we do not have use an optimized clear value for constant buffers
      IID_PPV_ARGS(mConstBufferUploadHeap[i].GetAddressOf()));
    mConstBufferUploadHeap[i]->SetName(L"Constant Buffer Upload Resource Heap");

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = mConstBufferUploadHeap[i]->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
    mDevice->CreateConstantBufferView(&cbvDesc, mConstDescHeap[i]->GetCPUDescriptorHandleForHeapStart());

    CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
    hr = mConstBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&mCbColorMultiplierAddr[i]));
    memcpy(mCbColorMultiplierAddr[i], &mCbColorMultiplierData, sizeof(mCbColorMultiplierData));
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
  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;        // format of the render target
  psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
  psoDesc.SampleDesc = sampleDesc;                           // must be the same sample description as the swapchain and depth/stencil buffer
  psoDesc.SampleMask = 0xffffffff;                           // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
  psoDesc.NumRenderTargets = 1; // we are only binding one render target
  psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // a default depth stencil state

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
  rootSigDesc.Init(mRootParams.size(), mRootParams.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

void BaseApp::InitDepth()
{
  // Create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
  dsvHeapDesc.NumDescriptors = 1;
  dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  HRESULT hr = mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsDescriptorHeap.GetAddressOf()));
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
  auto rdesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, mWidth, mHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
  mDevice->CreateCommittedResource(
    &hprop,
    D3D12_HEAP_FLAG_NONE,
    &rdesc,
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &depthOptimizedClearValue,
    IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
  );
  mDsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

  mDevice->CreateDepthStencilView(
    mDepthStencilBuffer.Get(), 
    &depthStencilDesc, 
    mDsDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
  );
}

D3D12_CPU_DESCRIPTOR_HANDLE BaseApp::DepthBufferView() const
{
  CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(mDsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
  return dsvHandle;
}