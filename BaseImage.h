#pragma once
#include <dxgi.h>
#include <wrl.h>
#include <wincodec.h>
#include <windows.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <vector>
#include <string>
#include "DebugOut.h"

using Microsoft::WRL::ComPtr;
using namespace std;


class BaseImage {
public:
  ComPtr<ID3D12Resource> mTextureBuffer;
  ComPtr<ID3D12Resource> mTextureUploadHeap;

  D3D12_RESOURCE_DESC mTextureDesc;
  int mImageBytesPerRow;
  int mImageSize;
  BYTE* mImageData;

  ~BaseImage();

  void AppendDescHeap(ID3D12Device* device, ID3D12DescriptorHeap* descHeap, int offset=0);
  void Read(const string& filepath);
  void Upload(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
};


// give reference to https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-textures-from-file

// get the dxgi format equivilent of a wic format
DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);

// get the number of bits per pixel for a dxgi format
int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

// load and decode image from file
int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC* resourceDescription, LPCWSTR filename, int* bytesPerRow);