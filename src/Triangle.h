#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <OGRE/OgrePrerequisites.h>

class TrianglePrivate;

class Triangle {
public:
  Triangle(const Ogre::Vector3 &p1, const Ogre::Vector3 &p2, const Ogre::Vector3 &p3,
           const Ogre::Vector2 &uv1, const Ogre::Vector2 &uv2, const Ogre::Vector2 &uv3,
           const Ogre::String materialName);
  ~Triangle();

  const Ogre::Vector3 position(const int i) const;
  const Ogre::Vector3 normal(const Ogre::Real u, const Ogre::Real v) const;
  const Ogre::Vector2 textureCoord(const Ogre::Real u, const Ogre::Real v) const;
  const Ogre::String materialName() const;

  const bool intersects(const Ogre::Ray &ray, Ogre::Real &t, Ogre::Real &u, Ogre::Real &v) const;
  const bool intersects(const Ogre::Ray &ray) const;

private:
  TrianglePrivate *d;
};

#endif // TRIANGLE_H
