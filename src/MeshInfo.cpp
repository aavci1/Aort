#include "MeshInfo.h"

#include "Triangle.h"

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreMath.h>
#include <OGRE/OgreMesh.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>

class MeshInfoPrivate {
public:
  MeshInfoPrivate() : entity(0), vertexCount(0), position(0), texCoord(0), indexCount(0), index(0), triangleCount(0), triangles(0) {
  }

  ~MeshInfoPrivate() {
    delete[] position;
    delete[] texCoord;
    delete[] index;
//    for (int i = 0; i < triangleCount; ++i)
//      delete triangles[i];
//    delete[] triangles;
  }

  const Ogre::Entity *entity;
  // vertices
  size_t vertexCount;
  Ogre::Vector3 *position;
  Ogre::Vector2 *texCoord;
  // indices
  size_t indexCount;
  Ogre::uint32 *index;
  // triangles
  size_t triangleCount;
  Triangle **triangles;
};

MeshInfo::MeshInfo(const Ogre::Entity *entity) : d(new MeshInfoPrivate()) {
  d->entity = entity;
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
  d->position = new Ogre::Vector3[d->vertexCount];
  d->texCoord = new Ogre::Vector2[d->vertexCount];
  d->index = new Ogre::uint32[d->indexCount];
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
    // get position data
    const Ogre::VertexElement *positionElement = mesh->sharedVertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
    if (positionElement) {
      Ogre::HardwareVertexBufferSharedPtr positionBuffer = mesh->sharedVertexData->vertexBufferBinding->getBuffer(positionElement->getSource());
      unsigned char *positionData = static_cast<unsigned char*>(positionBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
      for (size_t j = 0; j < mesh->sharedVertexData->vertexCount; ++j) {
        float *start = 0;
        // get position data for this vertex
        positionElement->baseVertexPointerToElement(positionData, &start);
        d->position[vertexOffset + j] = (orientation * (Ogre::Vector3(start[0], start[1], start[2]) * scale)) + position;
        // advance by vertex size
        positionData += positionBuffer->getVertexSize();
      }
      positionBuffer->unlock();
    }
    // get texture coordinate data
    const Ogre::VertexElement *texCoordElement = mesh->sharedVertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
    if (texCoordElement) {
      Ogre::HardwareVertexBufferSharedPtr texCoordBuffer = mesh->sharedVertexData->vertexBufferBinding->getBuffer(texCoordElement->getSource());
      unsigned char *texCoordData = static_cast<unsigned char*>(texCoordBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
      for (size_t j = 0; j < mesh->sharedVertexData->vertexCount; ++j) {
        float *start = 0;
        // get texture coordinate data for this vertex
        texCoordElement->baseVertexPointerToElement(texCoordData, &start);
        d->texCoord[vertexOffset + j] = Ogre::Vector2(start[0], start[1]);
        // advance by vertex size
        texCoordData += texCoordBuffer->getVertexSize();
      }
      texCoordBuffer->unlock();
    }
    vertexOffset += mesh->sharedVertexData->vertexCount;
  }
  // process submeshes
  for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
    Ogre::SubMesh *subMesh = mesh->getSubMesh(i);
    // extract vertices if not using shared vertices
    if (!subMesh->useSharedVertices) {
      // get position data
      const Ogre::VertexElement *positionElement = subMesh->vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
      if (positionElement) {
        Ogre::HardwareVertexBufferSharedPtr positionBuffer = subMesh->vertexData->vertexBufferBinding->getBuffer(positionElement->getSource());
        unsigned char *positionData = static_cast<unsigned char*>(positionBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        for (size_t j = 0; j < subMesh->vertexData->vertexCount; ++j) {
          float *start = 0;
          // get position data for this vertex
          positionElement->baseVertexPointerToElement(positionData, &start);
          d->position[vertexOffset + j] = (orientation * (Ogre::Vector3(start[0], start[1], start[2]) * scale)) + position;
          // advance by vertex size
          positionData += positionBuffer->getVertexSize();
        }
        positionBuffer->unlock();
      }
      // get texture coordinate data
      const Ogre::VertexElement *texCoordElement = subMesh->vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
      if (texCoordElement) {
        Ogre::HardwareVertexBufferSharedPtr texCoordBuffer = subMesh->vertexData->vertexBufferBinding->getBuffer(texCoordElement->getSource());
        unsigned char *texCoordData = static_cast<unsigned char*>(texCoordBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        for (size_t j = 0; j < subMesh->vertexData->vertexCount; ++j) {
          float *start = 0;
          // get texture coordinate data for this vertex
          texCoordElement->baseVertexPointerToElement(texCoordData, &start);
          d->texCoord[vertexOffset + j] = Ogre::Vector2(start[0], start[1]);
          // advance by vertex size
          texCoordData += texCoordBuffer->getVertexSize();
        }
        texCoordBuffer->unlock();
      }
    }
    // extract indices
    Ogre::HardwareIndexBufferSharedPtr indexBuffer = subMesh->indexData->indexBuffer;
    if (indexBuffer->getType() == Ogre::HardwareIndexBuffer::IT_32BIT) {
      Ogre::uint32 *indexData = static_cast<Ogre::uint32 *>(indexBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
      for (size_t j = 0; j < subMesh->indexData->indexCount; ++j)
        d->index[indexOffset + j] = indexData[subMesh->indexData->indexStart + j] + (subMesh->useSharedVertices ? 0 : vertexOffset);
      indexBuffer->unlock();
    } else {
      Ogre::uint16 *indexData = static_cast<Ogre::uint16 *>(indexBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
      for (size_t j = 0; j < subMesh->indexData->indexCount; ++j)
        d->index[indexOffset + j] = indexData[subMesh->indexData->indexStart + j] + (subMesh->useSharedVertices ? 0 : vertexOffset);
      indexBuffer->unlock();
    }
    // fill in the triangles array
    for (int j = 0; j < subMesh->indexData->indexCount / 3; ++j)
      d->triangles[triangleOffset + j] = new Triangle(d->position[indexOffset + d->index[j * 3 + 0]], d->position[indexOffset + d->index[j * 3 + 1]], d->position[indexOffset + d->index[j * 3 + 2]],
          d->texCoord[d->index[indexOffset + j * 3 + 0]], d->texCoord[indexOffset + d->index[j * 3 + 1]], d->texCoord[indexOffset + d->index[j * 3 + 2]],
          subMesh->getMaterialName());
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

Triangle **MeshInfo::triangles() {
  return d->triangles;
}
