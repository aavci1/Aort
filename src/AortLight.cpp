#include "AortLight.h"

#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>

namespace Aort {
  class LightPrivate {
  public:
    LightPrivate() : type(LT_POINT), diffuseColour(1.0f, 1.0f, 1.0f), specularColour(0.0f, 0.0f, 0.0f), size(100.0f, 100.0f) {
    }
    ~LightPrivate() {
    }

    LightType type;
    Ogre::ColourValue diffuseColour;
    Ogre::ColourValue specularColour;
    Ogre::Vector3 position;
    Ogre::Vector3 direction;
    Ogre::Vector2 size;
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
  }

  const Ogre::Vector3 &Light::getDirection() const {
    return d->direction;
  }

  void Light::setDirection(const Ogre::Vector3 &direction) {
    d->direction = direction;
  }

  const Ogre::Vector2 &Light::getSize() {
    return d->size;
  }

  void Light::setSize(const Ogre::Vector2 &size) {
    d->size = size;
  }
}
