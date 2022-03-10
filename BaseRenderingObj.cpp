#include "BaseRenderingObj.h"
#include "MathUtils.h"

void BaseRenderingObj::SetPos(float x, float y, float z)
{
  mPosition = XMFLOAT4(x, y, z, 0.0f);
}

XMMATRIX BaseRenderingObj::GetWorldMatrix()
{
  XMFLOAT4X4 I = Identity4x4();
  XMMATRIX ret = XMLoadFloat4x4(&I);

  XMVECTOR posVec = XMLoadFloat4(&mPosition); // create xmvector for  position
  XMMATRIX tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from position vector

  ret = tmpMat * ret;

  return ret;
}