#ifndef AORTMATERIAL_H
#define AORTMATERIAL_H

#include <OGRE/OgrePrerequisites.h>

namespace Aort {
  class Texture;

  class MaterialPrivate;

  class Material {
  public:
    Material(const Ogre::String &name);
    ~Material();

    void setName(const Ogre::String &name);
    const Ogre::String getName();

    void setAmbient(const Ogre::ColourValue &ambient);
    const Ogre::ColourValue getAmbient() const;

    void setDiffuse(const Ogre::ColourValue &diffuse);
    const Ogre::ColourValue getDiffuse() const;

    void setSpecular(const Ogre::ColourValue &specular);
    const Ogre::ColourValue getSpecular() const;

    void setShininess(const Ogre::Real &shininess);
    const Ogre::Real getShininess() const;

    void setReflectivity(const Ogre::Real &reflectivity);
    const Ogre::Real getReflectivity() const;

    void setTexture(Texture *texture);
    const Texture *getTexture() const;

    const Ogre::ColourValue getColourAt(const Ogre::Vector2 &uv) const;

  private:
    MaterialPrivate *d;
  };
}

#endif // AORTMATERIAL_H
