#include "AortMaterial.h"

#include "AortTexture.h"

#include <OGRE/OgreColourValue.h>

namespace Aort {
  class MaterialPrivate {
  public:
    MaterialPrivate() : ambient(1.0f, 1.0f, 1.0f), diffuse(1.0f, 1.0f, 1.0f), specular(0.0f, 0.0f, 0.0f), shininess(0.0f), reflectivity(0.25f), texture(0) {
    }

    ~MaterialPrivate() {
    }

    Ogre::String name;
    Ogre::ColourValue ambient;
    Ogre::ColourValue diffuse;
    Ogre::ColourValue specular;
    Ogre::Real shininess;
    Ogre::Real reflectivity;
    Texture *texture;
  };

  Material::Material(const Ogre::String &name) : d(new MaterialPrivate()) {
    d->name = name;
  }

  Material::~Material() {
    delete d;
  }

  void Material::setName(const Ogre::String &name) {
    d->name = name;
  }

  const Ogre::String Material::getName() {
    return d->name;
  }

  const Ogre::ColourValue Material::getAmbient() const {
    return d->ambient;
  }

  void Material::setAmbient(const Ogre::ColourValue &ambient) {
    d->ambient = ambient;
  }

  const Ogre::ColourValue Material::getDiffuse() const {
    return d->diffuse;
  }

  void Material::setDiffuse(const Ogre::ColourValue &diffuse) {
    d->diffuse = diffuse;
  }

  void Material::setSpecular(const Ogre::ColourValue &specular) {
    d->specular = specular;
  }

  const Ogre::ColourValue Material::getSpecular() const {
    return d->specular;
  }

  void Material::setShininess(const Ogre::Real &shininess) {
    d->shininess = shininess;
  }

  const Ogre::Real Material::getShininess() const {
    return d->shininess;
  }

  void Material::setReflectivity(const Ogre::Real &reflectivity) {
    d->reflectivity = reflectivity;
  }

  const Ogre::Real Material::getReflectivity() const {
    return d->reflectivity;
  }

  void Material::setTexture(Texture *texture) {
    d->texture = texture;
  }

  const Texture *Material::getTexture() const {
    return d->texture;
  }

  const Ogre::ColourValue Material::getColourAt(const Ogre::Vector2 &uv) const {
    if (d->texture)
      return d->texture->getColourAt(uv);
    return d->diffuse;
  }
}
