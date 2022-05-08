#include "BaseApp.h"
#include "../DebugOut.h"
#include <D3DCompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "../ThirdParty/ImGUI/imgui.h"
#include "../ThirdParty/ImGUI/imgui_impl_win32.h"
#include "../ThirdParty/ImGUI/imgui_impl_dx12.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;

BaseApp* BaseApp::mApp = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK BaseApp::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  if (msg == WM_DESTROY)
  {
    PostQuitMessage(0);
    return 0;
  } else if (msg == WM_SIZE) {
    BaseApp* app = BaseApp::mApp;
    if (app)
    {
        app->OnResize();
    }

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

  InitRTV();

  mMainHeap = new BaseMainHeap(mDevice.Get());
  mRuntimeHeap = new BaseRuntimeHeap(mDevice.Get());
  mUIRuntimeHeap = new BaseRuntimeHeap(mDevice.Get(), 1 + mFrameCnt);
  mGOManager = new BaseGameObjectManager(mDevice.Get(), mMainHeap);

  InitImGUI();

  mMainCamera = new BaseCamera(mDevice.Get(), mUIRuntimeHeap, static_cast<float>(mWidth), static_cast<float>(mHeight));
  mMainCamera->RegisterMainHandle(mMainHeap);

  mApp = this;
}

BaseApp::~BaseApp()
{
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  delete mMainCamera;
  delete mGOManager;
  delete mMainHeap;
  delete mRuntimeHeap;
  delete mUIRuntimeHeap;
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

  Start();
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

    // Update ticker
    mTimeDelta = float(clock() - mTimer) / CLOCKS_PER_SEC;
    mTimer = clock();

    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    Update();
    Render();
  }
}

void BaseApp::OnResize()
{
    // flush
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

    for (int i = 0; i < mFrameCnt; ++i)
    {
        mRenderTargets[i].Reset();
    }

    RECT rect = { 0, 0, 0, 0 };
    GetClientRect(mHwnd, &rect);
    mHeight = rect.bottom;
    mWidth = rect.right;

    ThrowIfFailed(mSwapChain->ResizeBuffers(
        mFrameCnt,
        mWidth, mHeight,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)
    );

    int rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (int i = 0; i < mFrameCnt; ++i)
    {
        HRESULT hr = mSwapChain->GetBuffer(
            i, IID_PPV_ARGS(mRenderTargets[i].GetAddressOf())
        );

        mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }
    mFrameIdx = 0;
}

void BaseApp::InitImGUI()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.Fonts->AddFontFromFileTTF("ThirdParty\\ImGUI\\Cousine-Regular.ttf", 16);

  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(mHwnd);


  // ImGUI would use start handle for font
  mUIRuntimeHeap->GetHeapHandleBlock(1); 
  // pick out first handle for sure (imGUI will use it!)
  ImGui_ImplDX12_Init(mDevice.Get(), mFrameCnt,
    DXGI_FORMAT_R8G8B8A8_UNORM, mUIRuntimeHeap->mDescHeap.Get(),
      mUIRuntimeHeap->mDescriptorHeapCPUStart,
      mUIRuntimeHeap->mDescriptorHeapGPUStart
  );
}

void BaseApp::Flush(ID3D12GraphicsCommandList* commandList)
{
  // Done recording commands.
  HRESULT hr = commandList->Close();
  ThrowIfFailed(hr);

  // Add the command list to the queue for execution.
  ID3D12CommandList* cmdsLists[] = { commandList };
  mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

  ++mExpectedFenceValue[mFrameIdx];
  hr = mCommandQueue->Signal(mFence[mFrameIdx].Get(), mExpectedFenceValue[mFrameIdx]);

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

}

void BaseApp::Enqueue(ID3D12GraphicsCommandList* commandList)
{
  // Done recording commands.
  HRESULT hr = commandList->Close();
  ThrowIfFailed(hr);

  // Add the command list to the queue for execution.
  ID3D12CommandList* cmdsLists[] = { commandList };
  mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

  if (FAILED(hr))
  {
    mIsRunning = false;
    return;
  }
}

void BaseApp::Swap() {
  // present the current backbuffer
  HRESULT hr = mSwapChain->Present(0, 0);
  if (FAILED(hr))
  {
    mIsRunning = false;
    return;
  }

  // frameIndex points to next backBuffer to be flipped to front-end
  mFrameIdx = mSwapChain.Get()->GetCurrentBackBufferIndex();
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

  // Client Rectangle is not exactly the same as mWidth and mHeight you specify
  // Windows 10 windows may vary a litte bit pixels
  // It should be aligned with GetClientRect
  RECT rect = { 0, 0, 0, 0 };
  GetClientRect(mHwnd, &rect);
  mHeight = rect.bottom;
  mWidth = rect.right;

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
