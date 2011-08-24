#ifndef AORTTRIANGLE_H
#define AORTTRIANGLE_H

#include <OGRE/OgrePrerequisites.h>

namespace Aort {
  class Material;

  class TrianglePrivate;

  class Triangle {
  public:
    Triangle(const Ogre::Vector3 &p1, const Ogre::Vector3 &p2, const Ogre::Vector3 &p3,
             const Ogre::Vector3 &n1, const Ogre::Vector3 &n2, const Ogre::Vector3 &n3,
             const Ogre::Vector2 &uv1, const Ogre::Vector2 &uv2, const Ogre::Vector2 &uv3,
             Material *material);
    ~Triangle();

    const Ogre::Vector3 position(const int i) const;
    const Ogre::Vector3 normal(const Ogre::Real u, const Ogre::Real v) const;
    const Ogre::Vector2 texCoord(const Ogre::Real u, const Ogre::Real v) const;
    const Material *getMaterial() const;

    const bool intersects(const Ogre::Ray &ray, Ogre::Real &t, Ogre::Real &u, Ogre::Real &v) const;

  private:
    TrianglePrivate *d;
  };
}

#endif // AORTTRIANGLE_H
