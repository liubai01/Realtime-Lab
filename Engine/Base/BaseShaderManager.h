#pragma once

#include <windows.h>
#include <wrl.h>
#include "../ThirdParty/d3dx12.h"
#include <string>
#include <memory>
#include <unordered_map>

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
  void AddVertexShader(std::string name, std::string entry = "main");
  void AddPixelShader(std::string name, std::string entry = "main");

  D3D12_SHADER_BYTECODE VertexShaderByteCode();
  D3D12_SHADER_BYTECODE PixelShaderByteCode();

private:
	std::unordered_map<std::string, std::unique_ptr<BaseShader>> mName2Shader;
};

