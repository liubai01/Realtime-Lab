#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#endif // !TINYOBJLOADER_IMPLEMENTATION

#include "BaseResourceMesh.h"
#include "../../DebugOut.h"

BaseResourceMesh::BaseResourceMesh(ID3D12Device* device, BaseAssetNode* assetNode) : BaseResource(device, assetNode)
{
    mType = BaseResourceMesh::ClassResourceType;

    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(assetNode->mFullPath, reader_config)) {
        if (!reader.Error().empty()) {
            dout::printf("[BaseResourceMesh]: %s", reader.Error().c_str());
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        dout::printf("[BaseResourceMesh]: ", reader.Warning().c_str());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        BaseResourceGeometry& geo = mGeos.emplace_back();
        geo.mName = shapes[s].name;
        // Loop over faces(polygon)
        // TBD: only support triangle now
        size_t index_offset = 0;
        
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            if (fv != 3)
            {
                dout::printf("face vertex != 3 found %d.\n", fv);
            }

            geo.mIndices.push_back(static_cast<DWORD>(0));
            geo.mIndices.push_back(static_cast<DWORD>(0));
            geo.mIndices.push_back(static_cast<DWORD>(0));
            // Loop over vertices in the face.
            for (size_t v = 0; v < 3; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = -attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                tinyobj::real_t nx = 1;
                tinyobj::real_t ny = 0;
                tinyobj::real_t nz = 0;
                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    nz = -attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                tinyobj::real_t tx = 0;
                tinyobj::real_t ty = 0;
                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                geo.mVertices.push_back({ {vx, vy, vz}, {nx, ny, nz}, {tx, 1.0f - ty} });

                geo.mIndices[geo.mIndices.size() - 1 - v] = static_cast<DWORD>(index_offset + v);
            }

            index_offset += fv;
        }
    }
}

BaseResourceMesh::~BaseResourceMesh()
{

}