#include "BaseShaderManager.h"
#include "../DebugOut.h"
#include <D3DCompiler.h>

#pragma region Shader

void BaseShaderManager::AddVertexShader(std::string name, std::string entry)
{
    std::unique_ptr<BaseShader> vertexShader(new BaseShader());
    ComPtr<ID3DBlob> errorBuff;

    std::wstring stemp = std::wstring(name.begin(), name.end());
    HRESULT hr = D3DCompileFromFile(stemp.c_str(),
        nullptr,
        nullptr,
        entry.c_str(),
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        vertexShader->shaderCPU.GetAddressOf(),
        errorBuff.GetAddressOf()
    );

  if (FAILED(hr))
  {
    OutputDebugStringA((char*)errorBuff->GetBufferPointer());
    return;
  }

  vertexShader->byteCode.BytecodeLength = vertexShader->shaderCPU->GetBufferSize();
  vertexShader->byteCode.pShaderBytecode = vertexShader->shaderCPU->GetBufferPointer();

  mName2Shader["VS"] = move(vertexShader);
}

void BaseShaderManager::AddPixelShader(std::string name, std::string entry)
{
    std::unique_ptr<BaseShader> pixelShader(new BaseShader());
  ComPtr<ID3DBlob> errorBuff;

  std::wstring stemp = std::wstring(name.begin(), name.end());
  HRESULT hr = D3DCompileFromFile(stemp.c_str(),
    nullptr,
    nullptr,
    entry.c_str(),
    "ps_5_0",
    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
    0,
    pixelShader->shaderCPU.GetAddressOf(),
    &errorBuff
  );

  if (FAILED(hr))
  {
    OutputDebugStringA((char*)errorBuff->GetBufferPointer());
    return;
  }

  D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
  pixelShader->byteCode.BytecodeLength = pixelShader->shaderCPU->GetBufferSize();
  pixelShader->byteCode.pShaderBytecode = pixelShader->shaderCPU->GetBufferPointer();

  mName2Shader["PS"] = move(pixelShader);
}

D3D12_SHADER_BYTECODE BaseShaderManager::VertexShaderByteCode()
{
  auto f = mName2Shader.find("VS");

  if (f != mName2Shader.end())
  {
    return mName2Shader["VS"]->byteCode;
  }

  dout::printf("Vertex shader not found! Please Use BaseApp::VertexShaderByteCode to find it. \n");
  return { nullptr, 0 };
}

D3D12_SHADER_BYTECODE BaseShaderManager::PixelShaderByteCode()
{
  auto f = mName2Shader.find("PS");

  if (f != mName2Shader.end())
  {
    return mName2Shader["PS"]->byteCode;
  }

  dout::printf("Pixel shader not found! Please Use BaseApp::PixelShaderByteCode to find it. \n");
  return { nullptr, 0 };
}

#pragma endregion