#ifndef AORTTEXTURE_H
#define AORTTEXTURE_H

#include <OGRE/OgreCommon.h>
#include <OGRE/OgrePrerequisites.h>

namespace Aort {
  class TexturePrivate;

  class Texture {
  public:
    Texture();
    ~Texture();

    void setImage(Ogre::Image *image);

    void setTransform(const Ogre::Matrix4 &transform);

    void setFilter(Ogre::FilterOptions filter);
    void setAnisotropy(const unsigned int anisotropy);

    const Ogre::ColourValue getColourAt(const Ogre::Vector2 &uv);

  private:
    TexturePrivate *d;
  };
}

#endif // AORTTEXTURE_H
