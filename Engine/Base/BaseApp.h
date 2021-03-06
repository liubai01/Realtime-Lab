#pragma once

#include <windows.h>
#include "../ThirdParty/d3dx12.h"
#include <wrl.h>
#include <vector>
#include <dxgi1_4.h>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "BaseShaderManager.h"
#include "../DebugOut.h"
#include "BaseGeometry.h"
#include "BaseObject.h"
#include "../MathUtils.h"
#include <time.h>
#include "BaseDrawContext.h"
#include "BaseCamera.h"
#include "BaseRuntimeHeap.h"
#include "BaseRenderTexture.h"
#include "BaseGameObjectManager.h"
#include "BaseProject.h"
#include "Resource/BaseResourceManager.h"
#include "BaseScene.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

class BaseApp
{
public:
	BaseApp(HINSTANCE hInstance, BaseProject* proj);
	virtual ~BaseApp();

	void Run();

	virtual void BeforeUpdate() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Start() {};

	void Enqueue(ID3D12GraphicsCommandList* commandList);
	void Flush(ID3D12GraphicsCommandList* commandList);
	void Swap();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void InitImGUI();
	void InitWindow(HINSTANCE hInstance);
	void InitDevice();
	void InitCommandQueue();
	void InitFence();
	void InitSwapChain();
	void InitRTV();

	void OnResize();

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

	int mFrameCnt = 3;
	int mFrameIdx = 0;
	bool mIsRunning = true;
	std::vector<ComPtr<ID3D12Fence>> mFence;
	std::vector<UINT64> mExpectedFenceValue;
	HANDLE mFenceEvent;

	ComPtr<ID3D12DescriptorHeap> mRtvDescriptorHeap;
	std::vector<ComPtr<ID3D12Resource>> mRenderTargets;

	int mWidth = 1440;
	int mHeight = 900;
	HWND mHwnd;

	clock_t mTimer;
	float mTimeDelta;

	ComPtr<IDXGIFactory4> mDxgiFactory;
	ComPtr<IDXGISwapChain3> mSwapChain;

	ComPtr<ID3D12Device> mDevice;

	ComPtr<ID3D12CommandQueue> mCommandQueue;

	BaseProject* mProject;

	BaseScene* mNowScene;
	BaseGameObjectManager* mGOManager;

	// The mainHeap / runtimeHeap manages the descriptors of resources
	// The descriptors in main would be copied to runtimeHeap if the resource is necessary
	BaseMainHeap* mMainHeap;
	BaseRuntimeHeap* mRuntimeHeap;


	static BaseApp* mApp;
};