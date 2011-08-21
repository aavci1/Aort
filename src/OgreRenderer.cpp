#include "OgreRenderer.h"

#include "KdTree.h"
#include "MeshInfo.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreLight.h>
#include <OGRE/OgreMovableObject.h>
#include <OGRE/OgreSceneNode.h>

#include <limits>

#ifndef NO_OMP
#include <omp.h>
#endif // !NO_OMP

class OgreRendererPrivate {
public:
  OgreRendererPrivate() : kdTree(0) {
  }

  ~OgreRendererPrivate() {
  }

  void traverse(Ogre::SceneNode *root) {
    for (int i = 0; i < root->numAttachedObjects(); ++i) {
      Ogre::MovableObject *object = root->getAttachedObject(i);
      // skip unvisible objects
      if (!object->isVisible())
        continue;
      // get object if it is an entity or light
      if (object->getMovableType() == "Entity")
        entities.append(static_cast<Ogre::Entity *>(object));
      else if (object->getMovableType() == "Light")
        lights.append(static_cast<Ogre::Light *>(object));
    }
    // traverse child nodes
    for (int i = 0; i < root->numChildren(); ++i)
      traverse(static_cast<Ogre::SceneNode *>(root->getChild(i)));
  }

  void buildTree() {
    QList<Triangle **> triangleList;
    size_t triangleCount = 0;
    Ogre::AxisAlignedBox aabb(Ogre::Vector3(0, 0, 0), Ogre::Vector3(0, 0, 0));
    // extract triangles from all meshes
    for (int i = 0; i < entities.size(); ++i) {
      // extend aabb
      aabb.merge(entities.at(i)->getWorldBoundingBox(true));
      // extract mesh info
      MeshInfo *meshInfo = new MeshInfo(entities.at(i));
      // add triangles to the list
      triangleList.append(meshInfo->triangles());
      // increase triangle count
      triangleCount += meshInfo->triangleCount();
      // clean up
      delete meshInfo;
    }
    // merge all triangles into a single list
    Triangle **triangles = new Triangle*[triangleCount + 1];
    size_t triangleIndex = 0;
    for (size_t i = 0; i < triangleList.size(); ++i) {
      for (size_t j = 0; triangleList.at(i)[j]; ++j)
        triangles[triangleIndex++] = triangleList.at(i)[j];
      // clean up
      delete[] triangleList.at(i);
    }
    // last triangle pointer in the list must be null
    triangles[triangleIndex] = 0;
    // build a kd-tree
    kdTree = new KdTreeNode(aabb, triangles);
  }

  Ogre::ColourValue traceRay(const Ogre::Ray &ray) {
    // trace ray using the kd-tree
    Triangle *triangle = 0;
    Ogre::Real t = FLT_MAX, u = 0, v = 0;
    if (kdTree->hit(ray, triangle, t, u, v))
      return Ogre::ColourValue::White;
    // TODO: return background color
    return Ogre::ColourValue::Black;
  }

  QList<Ogre::Entity *> entities;
  QList<Ogre::Light *> lights;
  KdTreeNode *kdTree;
};

OgreRenderer::OgreRenderer(QObject *parent) : QObject(parent), d(new OgreRendererPrivate()) {
}

OgreRenderer::~OgreRenderer() {
  delete d;
}

QImage OgreRenderer::render(Ogre::SceneNode *root, const Ogre::Camera *camera, const int width, const int height) {
  // extract entities and lights
  d->traverse(root);
  // build tree
  d->buildTree();
  // create empty image
  QImage result = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
  // precalculate 1/width and 1/height
  Ogre::Real inverseWidth = 1.0f / width;
  Ogre::Real inverseHeight = 1.0f / height;
#ifndef NO_OMP
  #pragma omp parallel for
#endif // !NO_OMP
  // start rendering
  for (int y = 0; y < height; ++y) {
    uchar *scanline = result.scanLine(y);
    for (int x = 0; x < width; ++x) {
      // create camera to viewport ray
      // and make sure that rays are not parallel to any axis
      Ogre::Ray ray = camera->getCameraToViewportRay(x * inverseWidth + std::numeric_limits<float>::epsilon(),
                                                     y * inverseHeight + std::numeric_limits<float>::epsilon());
      // trace the ray
      Ogre::ColourValue colour = d->traceRay(ray);
      // update image
      scanline[x * 4 + 0] = colour.r * 255;
      scanline[x * 4 + 1] = colour.g * 255;
      scanline[x * 4 + 2] = colour.b * 255;
      scanline[x * 4 + 3] = colour.a * 255;
    }
  }
  // free memory
  delete d->kdTree;
  // clean up
  d->entities.clear();
  d->lights.clear();
  d->kdTree = 0;
  // return result
  return result;
}
