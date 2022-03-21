#include "CoreGeometry.h"

void CoreGeometry::ComputeNormal()
{
  // Traverse all faces
  for (size_t i = 0; i < mIndices.size() / 3; ++i)
  {
    // indice of each triangle in this surface
    UINT i0 = mIndices[i * 3 + 0];
    UINT i1 = mIndices[i * 3 + 1];
    UINT i2 = mIndices[i * 3 + 2];

    // vertices of ith triangle
    CoreVertex& v0 = mVertices[i0];
    CoreVertex& v1 = mVertices[i1];
    CoreVertex& v2 = mVertices[i2];

    // compute face normal
    XMVECTOR v0_Pos = XMLoadFloat3(&v0.pos);
    XMVECTOR v0_Normal = XMLoadFloat3(&v0.normal);
    XMVECTOR v1_Pos = XMLoadFloat3(&v1.pos);
    XMVECTOR v1_Normal = XMLoadFloat3(&v1.normal);
    XMVECTOR v2_Pos = XMLoadFloat3(&v2.pos);
    XMVECTOR v2_Normal = XMLoadFloat3(&v2.normal);

    XMVECTOR e0 = v1_Pos - v0_Pos;
    XMVECTOR e1 = v2_Pos - v0_Pos;
    XMVECTOR faceNormal = XMVector3Normalize(XMVector3Cross(e0, e1));

    v0_Normal += faceNormal;
    v1_Normal += faceNormal;
    v2_Normal += faceNormal;

    XMStoreFloat3(&v0.normal, v0_Normal);
    XMStoreFloat3(&v1.normal, v1_Normal);
    XMStoreFloat3(&v2.normal, v2_Normal);
  }

  for (size_t i = 0; i < mVertices.size(); ++i)
  {
    XMVECTOR v_Normal = XMLoadFloat3(&mVertices[i].normal);
    v_Normal = XMVector3Normalize(v_Normal);
    XMStoreFloat3(&mVertices[i].normal, v_Normal);
  }
}