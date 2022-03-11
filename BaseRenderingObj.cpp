#include "BaseRenderingObj.h"
#include "MathUtils.h"

void BaseRenderingObj::SetPos(float x, float y, float z)
{
  mPosition = XMFLOAT4(x, y, z, 0.0f);
}

void BaseRenderingObj::SetRot(float x, float y, float z)
{
  mRotation = XMFLOAT4(x, y, z, 0.0f);
}

void BaseRenderingObj::SetScale(float x, float y, float z)
{
  mScale = XMFLOAT4(x, y, z, 0.0f);
}

XMMATRIX BaseRenderingObj::GetWorldMatrix()
{
  XMFLOAT4X4 tmp = Identity4x4();
  XMMATRIX ret = XMLoadFloat4x4(&tmp);
  XMMATRIX tmpMat;

  // rotation
  tmpMat = XMMatrixRotationX(mRotation.x);
  ret = tmpMat * ret;
  tmpMat = XMMatrixRotationY(mRotation.y);
  ret = tmpMat * ret;
  tmpMat = XMMatrixRotationZ(mRotation.z);
  ret = tmpMat * ret;

  // scale
  tmp = XMFLOAT4X4(
    mScale.x, 0.0f, 0.0f, 0.0f,
    0.0f, mScale.y, 0.0f, 0.0f,
    0.0f, 0.0f, mScale.z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  );
  tmpMat = XMLoadFloat4x4(&tmp);
  ret = tmpMat * ret;

  // translation
  XMVECTOR posVec = XMLoadFloat4(&mPosition); // create xmvector for  position
  tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from position vector
  ret = tmpMat * ret;

  return ret;
}

XMMATRIX BaseRenderingObj::GetRSInvT()
{
  XMFLOAT4X4 tmp = Identity4x4();
  XMMATRIX ret = XMLoadFloat4x4(&tmp);
  XMMATRIX tmpMat;

  // rotation
  tmpMat = XMMatrixRotationX(mRotation.x);
  ret = tmpMat * ret;
  tmpMat = XMMatrixRotationY(mRotation.y);
  ret = tmpMat * ret;
  tmpMat = XMMatrixRotationZ(mRotation.z);
  ret = tmpMat * ret;

  // scale
  tmp = XMFLOAT4X4(
    mScale.x, 0.0f, 0.0f, 0.0f,
    0.0f, mScale.y, 0.0f, 0.0f,
    0.0f, 0.0f, mScale.z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  );
  tmpMat = XMLoadFloat4x4(&tmp);
  ret = tmpMat * ret;

  ret = XMMatrixInverse(nullptr, ret);
  ret = XMMatrixTranspose(ret);

  return ret;
}