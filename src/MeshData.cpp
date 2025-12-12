

#include "MeshData.h"


void MeshData::clear(){
    vertices.clear();
    indices.clear();
}

size_t MeshData::verticesCount() const{
    return vertices.size();
}

size_t MeshData::indicesCount() const{
    return indices.size();
}