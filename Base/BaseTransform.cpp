#include "BaseTransform.h"
#include "../MathUtils.h"

BaseTransform::BaseTransform(ID3D12Device* device) : BaseStagedBuffer(device)
{
}

void BaseTransform::Upload()
{
  XMStoreFloat4x4(&mBuffer.mData.World, XMMatrixTranspose(GetWorldMatrix()));
  XMStoreFloat4x4(&mBuffer.mData.RSInvT, XMMatrixTranspose(GetRSInvT()));
}

void BaseTransform::SetPos(float x, float y, float z)
{
  mPosition = XMFLOAT4(x, y, z, 0.0f);
}

void BaseTransform::SetRot(float x, float y, float z)
{
  mRotation = XMFLOAT4(x, y, z, 0.0f);
}

void BaseTransform::SetScale(float x, float y, float z)
{
  mScale = XMFLOAT4(x, y, z, 0.0f);
}

XMMATRIX BaseTransform::GetWorldMatrix()
{
  XMFLOAT4X4 tmp = Identity4x4();
  XMMATRIX ret = XMLoadFloat4x4(&tmp);
  XMMATRIX tmpMat;

  // translation
  XMVECTOR posVec = XMLoadFloat4(&mPosition); // create xmvector for  position
  tmpMat = XMMatrixTranslationFromVector(posVec); // create translation matrix from position vector
  ret = tmpMat * ret;

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

  return ret;
}

XMMATRIX BaseTransform::GetRSInvT()
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