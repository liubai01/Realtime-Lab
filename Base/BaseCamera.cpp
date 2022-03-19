#include "BaseCamera.h"

BaseCamera::BaseCamera(float width, float height, float FovAngleY, float nearZ, float FarZ)
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

  XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * 3.1415926535f, static_cast<float>(width) / height, 1.0f, 1000.0f);
  XMStoreFloat4x4(&mProj, P);
}