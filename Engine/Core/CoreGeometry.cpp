#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#endif // !TINYOBJLOADER_IMPLEMENTATION

#include "CoreGeometry.h"

CoreGeometry::CoreGeometry()
{
}

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


void CoreGeometry::ComputeTangentSpace()
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
        XMVECTOR v1_Pos = XMLoadFloat3(&v1.pos);
        XMVECTOR v2_Pos = XMLoadFloat3(&v2.pos);

        XMVECTOR e1 = v1_Pos - v0_Pos;
        XMVECTOR e2 = v2_Pos - v1_Pos;

        float delta_u1 = v1.texCoord.x - v0.texCoord.x;
        float delta_u2 = v2.texCoord.x - v1.texCoord.x;

        float delta_v1 = v1.texCoord.y - v0.texCoord.y;
        float delta_v2 = v2.texCoord.y - v1.texCoord.y;

        float r = delta_u1 * delta_v2 - delta_u2 * delta_v1;

        XMVECTOR T = XMVector3Normalize((delta_v2 * e1 - delta_v1 * e2));
        //XMVECTOR B = (delta_u1 * e2 - delta_u2 * e1) / r;
        XMVECTOR B = XMVector3Normalize((delta_u1 * e2 - delta_u2 * e1));
        XMVECTOR N = XMLoadFloat3(&v1.normal);

        ////XMVECTOR tmp = delta_u2 * T + delta_v2 * B - e2;6
        //XMVECTOR tmp = XMVector3Dot(T, N);
        //XMVECTOR tmp = T;
        //XMFLOAT3 tmp2;
        //XMStoreFloat3(&tmp2, tmp);

        //dout::printf("%f %f %f\n", tmp2.x, tmp2.y, tmp2.z);
        //dout::printf("u1: %f, u2: %f, v1: %f, v2: %f\n", delta_u1, delta_u2, delta_v1, delta_v2);



        XMStoreFloat3(&v0.tangent, T);
        XMStoreFloat3(&v0.bitangent, B);
        XMStoreFloat3(&v1.tangent, T);
        XMStoreFloat3(&v1.bitangent, B);
        XMStoreFloat3(&v2.tangent, T);
        XMStoreFloat3(&v2.bitangent, B);
    }

}

