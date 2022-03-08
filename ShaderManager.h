#pragma once

#include <windows.h>
#include <wrl.h>
#include "d3dx12.h"
#include <string>
#include <memory>
#include <unordered_map>

using namespace std;
using Microsoft::WRL::ComPtr;

class Shader
{
public:
  ComPtr<ID3DBlob> shaderCPU;
  D3D12_SHADER_BYTECODE byteCode;
};

class ShaderManager
{
public:
  void AddVertexShader(string name, string entry = "main");
  void AddPixelShader(string name, string entry = "main");

  D3D12_SHADER_BYTECODE VertexShaderByteCode();
  D3D12_SHADER_BYTECODE PixelShaderByteCode();

private:
  unordered_map<string, unique_ptr<Shader>> mName2Shader;
};

