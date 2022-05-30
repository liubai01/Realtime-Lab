#pragma once

#include <windows.h>
#include <wrl.h>
#include "../ThirdParty/d3dx12.h"
#include <string>
#include <memory>
#include <unordered_map>
#include "Asset/BaseAssetManager.h"

using Microsoft::WRL::ComPtr;

class BaseShader
{
public:
  ComPtr<ID3DBlob> shaderCPU;
  D3D12_SHADER_BYTECODE byteCode;
};

class BaseShaderManager
{
public:
	BaseShaderManager(BaseAssetManager* assetManager);

	void AddVertexShader(std::string url, std::string entry = "main");
	void AddPixelShader(std::string url, std::string entry = "main");

	D3D12_SHADER_BYTECODE VertexShaderByteCode();
	D3D12_SHADER_BYTECODE PixelShaderByteCode();

private:
	BaseAssetManager* mAssetManager;
	std::unordered_map<std::string, std::unique_ptr<BaseShader>> mName2Shader;
};

