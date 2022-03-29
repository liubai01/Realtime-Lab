#include "BaseCamera.h"

BaseCamera::BaseCamera(ID3D12Device* device, float width, float height, float FovAngleY, float nearZ, float FarZ) : BaseStagedBuffer(device)
{
  mWidth = width;
  mHeight = height;

  // Fill out the Viewport
  mViewport.TopLeftX = 0;
  mViewport.TopLeftY = 0;
  mViewport.Width = static_cast<float>(width);
  mViewport.Height = static_cast<float>(height);
  mViewport.MinDepth = 0.0f;
  mViewport.MaxDepth = 1.0f;

  // Fill out a scissor rect
  mScissorRect.left = 0;
  mScissorRect.top = 0;
  mScissorRect.right = static_cast<long>(width);
  mScissorRect.bottom = static_cast<long>(height);

  XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, static_cast<float>(width) / height, nearZ, FarZ);
  XMStoreFloat4x4(&mProj, P);

  XMVECTOR pos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
  XMStoreFloat4(&mPos, pos);
}

void BaseCamera::SetPos(float x, float y, float z)
{
    mPos.x = x;
    mPos.y = y;
    mPos.z = z;
}

void BaseCamera::Upload()
{
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR pos = XMLoadFloat4(&mPos);
    
    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);

    XMMATRIX viewproj = view * proj;

    XMStoreFloat4x4(&mBuffer.mData.ViewProj, XMMatrixTranspose(viewproj));
    mBuffer.mData.EyePos = mPos;
}