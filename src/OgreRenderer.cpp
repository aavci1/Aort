#include "OgreRenderer.h"

#include "KdTree.h"
#include "MeshInfo.h"
#include "Triangle.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreLight.h>
#include <OGRE/OgreMovableObject.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>

#include <limits>

#ifndef NO_OMP
#include <omp.h>
#endif // !NO_OMP

#define EPSILON 0.001f

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

  Ogre::ColourValue traceRay(const Ogre::Ray &ray, int depth = 0) {
    // TODO: initialize color as background color
    Ogre::ColourValue finalColour = Ogre::ColourValue::Black;
    // trace ray using the kd-tree
    Triangle *triangle = 0;
    Ogre::Real t = FLT_MAX, u = 0, v = 0;
    // calculate view vector
    Ogre::Vector3 V = ray.getDirection();
    if (kdTree->hit(ray, triangle, t, u, v)) {
      // add ambient lighting
      finalColour += calculateAmbient() * triangle->getAmbientColour(u, v);
      // calculate hit point
      Ogre::Vector3 P = ray.getPoint(t - EPSILON);
      // calculate triangle normal
      Ogre::Vector3 N = triangle->normal(u, v);
      Ogre::ColourValue diffuseColour = triangle->getDiffuseColour(u, v);
      Ogre::ColourValue specularColour = triangle->getSpecularColour(u, v);
      for (int i = 0; i < lights.size(); ++i) {
        // calculate light vector
        Ogre::Vector3 L = lights.at(i)->getDerivedPosition() - P;
        Ogre::Real length = L.normalise();
        // calculate shade
        Ogre::Real shade = calculateShade(P, L, length);
        if (shade > std::numeric_limits<float>::epsilon()) {
          // add diffuse
          finalColour += shade * calculateDiffuse(N, L) * diffuseColour * lights.at(i)->getDiffuseColour();
          // add specular
          finalColour += shade * calculateSpecular(V, N, L) * specularColour * lights.at(i)->getSpecularColour();
        }
      }
      // TODO: make maxDepth configurable
      int maxDepth = 3;
      // add reflections
      if (depth < maxDepth)
        finalColour += triangle->getReflectivity() * calculateReflection(P, V, N, depth) * diffuseColour;
    }
    // set full opacity
    finalColour.r = Ogre::Math::Clamp(finalColour.r, 0.0f, 1.0f);
    finalColour.g = Ogre::Math::Clamp(finalColour.g, 0.0f, 1.0f);
    finalColour.b = Ogre::Math::Clamp(finalColour.b, 0.0f, 1.0f);
    finalColour.a = 1.0f;
    // return final color
    return finalColour;
  }

  Ogre::Real calculateShade(const Ogre::Vector3 &P, const Ogre::Vector3 &L, Ogre::Real length) {
    // TODO: implement area lights and thus soft shadows
    if (kdTree->hit(Ogre::Ray(P, L), EPSILON, length))
      return 0.0f;
    return 1.0f;
  }

  Ogre::ColourValue calculateAmbient() {
    return ambientColour;
  }

  Ogre::Real calculateDiffuse(const Ogre::Vector3 &N, const Ogre::Vector3 &L) {
    float dot = N.dotProduct(L);
    if (dot > 0.0f)
      return dot;
    return 0;
  }

  Ogre::Real calculateSpecular(const Ogre::Vector3 &V, const Ogre::Vector3 &N, const Ogre::Vector3 &L) {
    // TODO: make specular configurable
    Ogre::Vector3 R = L - 2.0f * N.dotProduct(L) * N;
    float dot = V.dotProduct(R);
    if (dot > 0)
      return  dot / (50 - 50 * dot + dot);
    return 0;
  }

  Ogre::ColourValue calculateReflection(const Ogre::Vector3 &P, const Ogre::Vector3 &V, const Ogre::Vector3 &N, int depth = 0) {
    // calculate reflection vector
    Ogre::Vector3 R = V - 2.0f * N.dotProduct(V) * N;
    // TODO: Implement diffuse (scattered) reflections
    return traceRay(Ogre::Ray(P + R * EPSILON, R), depth + 1);
  }

  QList<Ogre::Entity *> entities;
  QList<Ogre::Light *> lights;
  KdTreeNode *kdTree;
  Ogre::ColourValue ambientColour;
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
  // set ambient colour
  d->ambientColour = Ogre::ColourValue(0.1f, 0.1f, 0.1f);
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
