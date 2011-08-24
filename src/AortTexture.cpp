#include "AortTexture.h"

#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreImage.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreVector2.h>

namespace Aort {
  class TexturePrivate {
  public:
    TexturePrivate() : image(0), width(0), height(0), transform(Ogre::Matrix4::IDENTITY), filter(Ogre::FO_NONE), anisotropy(1) {
    }

    ~TexturePrivate() {
      delete image;
    }

    Ogre::Image *image;
    size_t width;
    size_t height;
    Ogre::Matrix4 transform;
    Ogre::FilterOptions filter;
    unsigned int anisotropy;
  };

  Texture::Texture() : d(new TexturePrivate()) {
  }

  Texture::~Texture() {
    delete d;
  }

  void Texture::setImage(Ogre::Image *image) {
    d->image = image;
    d->width = image->getWidth();
    d->height = image->getHeight();
  }

  void Texture::setTransform(const Ogre::Matrix4 &transform) {
    d->transform = transform;
  }

  void Texture::setFilter(Ogre::FilterOptions filter) {
    d->filter = filter;
  }

  void Texture::setAnisotropy(const unsigned int anisotropy) {
    d->anisotropy = anisotropy;
  }

  const Ogre::ColourValue Texture::getColourAt(const Ogre::Vector2 &uv) {
    // return white if image is null
    if (!d->image)
      return Ogre::ColourValue::White;
    // initialize result as white
    Ogre::ColourValue result = Ogre::ColourValue::White;
    // transform the texture coordinates
    Ogre::Real u = (d->transform[0][0] * uv.x + d->transform[1][0] * uv.y + d->transform[2][0]) * d->width;
    Ogre::Real v = (d->transform[0][1] * uv.x + d->transform[1][1] * uv.y + d->transform[2][1]) * d->height;
    if (d->filter == Ogre::FO_POINT) {
      // nearest point filtering
      int x = int(floorf(u + 0.5f)) % d->width;
      int y = int(floorf(u + 0.5f)) % d->height;
      result = d->image->getColourAt(x, y, 0);
    } else if (d->filter == Ogre::FO_LINEAR) {
      // bilinear filtering
      int u1 = int(u) % d->width;
      int v1 = int(v) % d->height;
      int u2 = (u1 + 1) % d->width;
      int v2 = (v1 + 1) % d->height;
      // calculate fractional parts of u and v
      Ogre::Real fracu = u - floorf(u);
      Ogre::Real fracv = v - floorf(v);
      // calculate weight factors
      Ogre::Real w1 = (1 - fracu) * (1 - fracv);
      Ogre::Real w2 = fracu * (1 - fracv);
      Ogre::Real w3 = (1 - fracu) * fracv;
      Ogre::Real w4 = fracu *  fracv;
      // fetch four texels
      Ogre::ColourValue c1 = d->image->getColourAt(u1, v1, 0);
      Ogre::ColourValue c2 = d->image->getColourAt(u2, v1, 0);
      Ogre::ColourValue c3 = d->image->getColourAt(u1, v2, 0);
      Ogre::ColourValue c4 = d->image->getColourAt(u2, v2, 0);
      // scale and sum the four colors
      result = c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
    } else if (d->filter == Ogre::FO_ANISOTROPIC) {
      // TODO: Implement anisotropic filtering
      // use bilinear filtering until anisotropic filtering is implemented
      int u1 = int(u) % d->width;
      int v1 = int(v) % d->height;
      int u2 = (u1 + 1) % d->width;
      int v2 = (v1 + 1) % d->height;
      // calculate fractional parts of u and v
      Ogre::Real fracu = u - floorf(u);
      Ogre::Real fracv = v - floorf(v);
      // calculate weight factors
      Ogre::Real w1 = (1 - fracu) * (1 - fracv);
      Ogre::Real w2 = fracu * (1 - fracv);
      Ogre::Real w3 = (1 - fracu) * fracv;
      Ogre::Real w4 = fracu *  fracv;
      // fetch four texels
      Ogre::ColourValue c1 = d->image->getColourAt(u1, v1, 0);
      Ogre::ColourValue c2 = d->image->getColourAt(u2, v1, 0);
      Ogre::ColourValue c3 = d->image->getColourAt(u1, v2, 0);
      Ogre::ColourValue c4 = d->image->getColourAt(u2, v2, 0);
      // scale and sum the four colors
      result = c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
    }
    // HACK: swap r and b components -- dont know why, but this generates correct results
    return Ogre::ColourValue(result.b, result.g, result.r);
  }
}
