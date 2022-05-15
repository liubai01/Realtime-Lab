#pragma once
#include "../Base/BaseGeometry.h"

struct CoreVertex {
  XMFLOAT3 pos;
  XMFLOAT3 normal;
  XMFLOAT2 texCoord;

  XMFLOAT3 tangent;
  XMFLOAT3 bitangent;
};


//#define CoreGeometry BaseGeometry<CoreVertex>

class CoreGeometry : public BaseGeometry<CoreVertex>
{
public:
  void ComputeNormal();

  void ComputeTangentSpace();

  static vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout()
  {
      vector<D3D12_INPUT_ELEMENT_DESC> ret =
      {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
          { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
          { "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
          { "BITANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
      };

      return ret;
  }
};

