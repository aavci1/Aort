#include "MeshInfo.h"

#include "Triangle.h"

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreMath.h>
#include <OGRE/OgreMesh.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>

class Vertex {
public:
  Vertex() : position(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 0.0f), texCoord(0.0f, 0.0f) {
  }

  Ogre::Vector3 position;
  Ogre::Vector3 normal;
  Ogre::Vector2 texCoord;
};

class MeshInfoPrivate {
public:
  MeshInfoPrivate() : vertexCount(0), vertices(0), indexCount(0), indices(0), triangleCount(0), triangles(0) {
  }

  ~MeshInfoPrivate() {
    delete[] vertices;
    delete[] indices;
//    for (int i = 0; i < triangleCount; ++i)
//      delete triangles[i];
//    delete[] triangles;
  }

  // vertices
  size_t vertexCount;
  Vertex *vertices;
  // indices
  size_t indexCount;
  Ogre::uint32 *indices;
  // triangles
  size_t triangleCount;
  Triangle **triangles;
};

void readPositions(Ogre::VertexData *vertexData, Vertex *vertices, size_t vertexOffset, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation, const Ogre::Vector3 &scale) {
  // get position data
  const Ogre::VertexElement *positionElement = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
  if (positionElement) {
    Ogre::HardwareVertexBufferSharedPtr positionBuffer = vertexData->vertexBufferBinding->getBuffer(positionElement->getSource());
    unsigned char *positionData = static_cast<unsigned char*>(positionBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
    for (size_t j = 0; j < vertexData->vertexCount; ++j) {
      float *start = 0;
      // get position data for this vertex
      positionElement->baseVertexPointerToElement(positionData, &start);
      vertices[vertexOffset + j].position = (orientation * (Ogre::Vector3(start[0], start[1], start[2]) * scale)) + position;
      // advance by vertex size
      positionData += positionBuffer->getVertexSize();
    }
    positionBuffer->unlock();
  }
}

void readNormals(Ogre::VertexData *vertexData, Vertex *vertices, size_t vertexOffset, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation, const Ogre::Vector3 &scale) {
  // get normal data
  const Ogre::VertexElement *normalElement = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
  if (normalElement) {
    Ogre::HardwareVertexBufferSharedPtr normalBuffer = vertexData->vertexBufferBinding->getBuffer(normalElement->getSource());
    unsigned char *normalData = static_cast<unsigned char*>(normalBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
    for (size_t j = 0; j < vertexData->vertexCount; ++j) {
      float *start = 0;
      // get normal data for this vertex
      normalElement->baseVertexPointerToElement(normalData, &start);
      vertices[vertexOffset + j].normal = (orientation * (Ogre::Vector3(start[0], start[1], start[2]) * scale)) + position;
      vertices[vertexOffset + j].normal.normalise();
      // advance by vertex size
      normalData += normalBuffer->getVertexSize();
    }
    normalBuffer->unlock();
  }
}

void readTexCoords(Ogre::VertexData *vertexData, Vertex *vertices, size_t vertexOffset) {
  // get texCoord data
  const Ogre::VertexElement *texCoordElement = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
  if (texCoordElement) {
    Ogre::HardwareVertexBufferSharedPtr texCoordBuffer = vertexData->vertexBufferBinding->getBuffer(texCoordElement->getSource());
    unsigned char *texCoordData = static_cast<unsigned char*>(texCoordBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
    for (size_t j = 0; j < vertexData->vertexCount; ++j) {
      float *start = 0;
      // get texCoord data for this vertex
      texCoordElement->baseVertexPointerToElement(texCoordData, &start);
      vertices[vertexOffset + j].texCoord = Ogre::Vector2(start[0], start[1]);
      // advance by vertex size
      texCoordData += texCoordBuffer->getVertexSize();
    }
    texCoordBuffer->unlock();
  }
}

MeshInfo::MeshInfo(const Ogre::Entity *entity) : d(new MeshInfoPrivate()) {
  // get position orientation and scale
  Ogre::Vector3 position = entity->getParentSceneNode()->_getDerivedPosition();
  Ogre::Quaternion orientation = entity->getParentSceneNode()->_getDerivedOrientation();
  Ogre::Vector3 scale = entity->getParentSceneNode()->_getDerivedScale();
  // extract mesh information
  bool useSharedVertices = false;
  // calculate total number of the vertices and indices in the mesh
  Ogre::Mesh *mesh = entity->getMesh().getPointer();
  for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
    Ogre::SubMesh *subMesh = mesh->getSubMesh(i);
    // add vertex count if not using shared vertices
    if (subMesh->useSharedVertices)
      useSharedVertices = true;
    else
      d->vertexCount += subMesh->vertexData->vertexCount;
    // add index count
    d->indexCount += subMesh->indexData->indexCount;
  }
  // add shared vertex count, if used
  if (useSharedVertices)
    d->vertexCount += mesh->sharedVertexData->vertexCount;
  // create buffers
  d->vertices = new Vertex[d->vertexCount];
  d->indices = new Ogre::uint32[d->indexCount];
  // calculate triangle count
  d->triangleCount = d->indexCount / 3;
  // create triangles array
  d->triangles = new Triangle*[d->triangleCount + 1];
  d->triangles[d->triangleCount] = 0;
  // offset of the first vertex or index position to write into
  size_t vertexOffset = 0;
  size_t indexOffset = 0;
  size_t triangleOffset = 0;
  // extract shared vertices
  if (useSharedVertices) {
    readPositions(mesh->sharedVertexData, d->vertices, vertexOffset, position, orientation, scale);
    readNormals(mesh->sharedVertexData, d->vertices, vertexOffset, position, orientation, scale);
    readTexCoords(mesh->sharedVertexData, d->vertices, vertexOffset);
    vertexOffset += mesh->sharedVertexData->vertexCount;
  }
  // process submeshes
  for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
    Ogre::SubMesh *subMesh = mesh->getSubMesh(i);
    // extract vertices if not using shared vertices
    if (!subMesh->useSharedVertices) {
      readPositions(subMesh->vertexData, d->vertices, vertexOffset, position, orientation, scale);
      readNormals(subMesh->vertexData, d->vertices, vertexOffset, position, orientation, scale);
      readTexCoords(subMesh->vertexData, d->vertices, vertexOffset);
    }
    // extract indices
    Ogre::HardwareIndexBufferSharedPtr indexBuffer = subMesh->indexData->indexBuffer;
    if (indexBuffer->getType() == Ogre::HardwareIndexBuffer::IT_32BIT) {
      Ogre::uint32 *indexData = static_cast<Ogre::uint32 *>(indexBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
      for (size_t j = 0; j < subMesh->indexData->indexCount; ++j)
        d->indices[indexOffset + j] = indexData[subMesh->indexData->indexStart + j] + (subMesh->useSharedVertices ? 0 : vertexOffset);
      indexBuffer->unlock();
    } else {
      Ogre::uint16 *indexData = static_cast<Ogre::uint16 *>(indexBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
      for (size_t j = 0; j < subMesh->indexData->indexCount; ++j)
        d->indices[indexOffset + j] = indexData[subMesh->indexData->indexStart + j] + (subMesh->useSharedVertices ? 0 : vertexOffset);
      indexBuffer->unlock();
    }
    // fill in the triangles array
    for (int j = 0; j < subMesh->indexData->indexCount / 3; ++j) {
      const Vertex &v1 = d->vertices[indexOffset + d->indices[j * 3 + 0]];
      const Vertex &v2 = d->vertices[indexOffset + d->indices[j * 3 + 1]];
      const Vertex &v3 = d->vertices[indexOffset + d->indices[j * 3 + 2]];
      d->triangles[triangleOffset + j] = new Triangle(v1.position, v2.position, v3.position, v1.normal, v2.normal, v3.normal, v1.texCoord, v2.texCoord, v3.texCoord, subMesh->getMaterialName());
    }
    // update vertex and index offsets
    if (!subMesh->useSharedVertices)
      vertexOffset += subMesh->vertexData->vertexCount;
    indexOffset += subMesh->indexData->indexCount;
    triangleOffset += subMesh->indexData->indexCount / 3;
  }
}

MeshInfo::~MeshInfo() {
  delete d;
}

const size_t MeshInfo::triangleCount() const {
  return d->triangleCount;
}

Triangle **MeshInfo::triangles() const {
  return d->triangles;
}
