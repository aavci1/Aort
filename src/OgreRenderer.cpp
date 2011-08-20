#include "OgreRenderer.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreLight.h>
#include <OGRE/OgreMovableObject.h>
#include <OGRE/OgreSceneNode.h>

class OgreRendererPrivate {
public:
  OgreRendererPrivate() {
  }

  ~OgreRendererPrivate() {
  }

  void traverse(Ogre::SceneNode *root) {
    // TODO: conceive an acceleration structure
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

  Ogre::ColourValue traceRay(const Ogre::Ray &ray) {
    // TODO: to actually trace the ray to the polygon level
    for (int i = 0; i < entities.size(); ++i) {
      if (ray.intersects(entities.at(i)->getWorldBoundingBox(true)).first)
        return Ogre::ColourValue::White;
    }
    // TODO: return background color
    return Ogre::ColourValue::Black;
  }

  QList<Ogre::Entity *> entities;
  QList<Ogre::Light *> lights;
};

OgreRenderer::OgreRenderer(QObject *parent) : QObject(parent), d(new OgreRendererPrivate()) {
}

OgreRenderer::~OgreRenderer() {
  delete d;
}

QImage OgreRenderer::render(Ogre::SceneNode *root, const Ogre::Camera *camera, const int width, const int height, const int maxReflection) {
  // extract entities and lights
  d->traverse(root);
  // create empty image
  QImage result = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
  // precalculate 1/width and 1/height
  float inverseWidth = 1.0f / width;
  float inverseHeight = 1.0f / height;
  // start rendering
  for (int y = 0; y < height; ++y) {
    uchar *scanline = result.scanLine(y);
    for (int x = 0; x < width; ++x) {
      // create camera to viewport ray
      Ogre::Ray ray = camera->getCameraToViewportRay(x * inverseWidth, y * inverseHeight);
      // trace the ray
      Ogre::ColourValue colour = d->traceRay(ray);
      // update image
      scanline[x * 4 + 0] = colour.r * 255;
      scanline[x * 4 + 1] = colour.g * 255;
      scanline[x * 4 + 2] = colour.b * 255;
      scanline[x * 4 + 3] = colour.a * 255;
    }
  }
  return result;
}

