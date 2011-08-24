#include "AortLight.h"

#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>

namespace Aort {
  class LightPrivate {
  public:
    LightPrivate() : type(LT_POINT), diffuseColour(1.0f, 1.0f, 1.0f), specularColour(0.0f, 0.0f, 0.0f), size(100.0f, 100.0f), modified(true) {
    }
    ~LightPrivate() {
    }

    LightType type;
    Ogre::ColourValue diffuseColour;
    Ogre::ColourValue specularColour;
    Ogre::Vector3 position;
    Ogre::Vector3 direction;
    Ogre::Vector2 size;
    Ogre::Real cellSizeX;
    Ogre::Real cellSizeY;
    Ogre::Vector3 grid[16];
    Ogre::Vector3 points[16];
    bool modified;
  };

  Light::Light() : d(new LightPrivate()) {
  }

  Light::~Light() {
    delete d;
  }

  const LightType &Light::getType() const {
    return d->type;
  }

  void Light::setType(const LightType &type) {
    d->type = type;
  }

  const Ogre::ColourValue &Light::getDiffuseColour() const {
    return d->diffuseColour;
  }

  void Light::setDiffuseColour(const Ogre::ColourValue &colour) {
    d->diffuseColour = colour;
  }

  const Ogre::ColourValue &Light::getSpecularColour() const {
    return d->specularColour;
  }

  void Light::setSpecularColour(const Ogre::ColourValue &colour) {
    d->specularColour = colour;
  }

  const Ogre::Vector3 &Light::getPosition() const {
    return d->position;
  }

  void Light::setPosition(const Ogre::Vector3 &position) {
    d->position = position;
    // set modified flag
    d->modified = true;
  }

  const Ogre::Vector3 &Light::getDirection() const {
    return d->direction;
  }

  void Light::setDirection(const Ogre::Vector3 &direction) {
    d->direction = direction;
    // set modified flag
    d->modified = true;
  }

  const Ogre::Vector2 &Light::getSize() const {
    return d->size;
  }

  void Light::setSize(const Ogre::Vector2 &size) {
    d->size = size;
    // set modified flag
    d->modified = true;
  }

  float randf() {
    return float(rand()) / float(RAND_MAX);
  }

  const Ogre::Vector3 *Light::getPoints() const {
    if (d->modified) {
      // calculate cell size
      d->cellSizeX = d->size.x * 0.25f;
      d->cellSizeY = d->size.y * 0.25f;
      // calculate top left point
      Ogre::Vector3 p1(d->position.x - d->size.x * 0.5f, d->position.y, d->position.z - d->size.y * 0.5f);
      // calculate top left of each cell
      for (int i = 0; i < 16; ++i)
        d->grid[i] = p1 + Ogre::Vector3((i / 4) * d->cellSizeX, 0, (i % 4) * d->cellSizeY);
      // unset modified flag
      d->modified = false;
    }
    // calculate points
    for (int i = 0; i < 16; ++i)
      d->points[i] = d->grid[i] + Ogre::Vector3(randf() * d->cellSizeX, 0.0f, randf() * d->cellSizeY);
    // return result
    return d->points;
  }
}
