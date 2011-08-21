#include "Triangle.h"

#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreMath.h>
#include <OGRE/OgreRay.h>
#include <OGRE/OgreString.h>
#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>

#include <float.h>
#include <math.h>

class TrianglePrivate {
public:
  TrianglePrivate() {}
  ~TrianglePrivate() {}

  Ogre::Vector3 position[3];
  Ogre::Vector3 normal;
  Ogre::Vector2 uv[3];
  Ogre::String materialName;
  // intersection
  Ogre::Vector3 b;
  Ogre::Vector3 c;
  Ogre::Real inv_det;
  int axis;
  int uAxis;
  int vAxis;
};

Triangle::Triangle(const Ogre::Vector3 &p1, const Ogre::Vector3 &p2, const Ogre::Vector3 &p3,
                   const Ogre::Vector2 &uv1, const Ogre::Vector2 &uv2, const Ogre::Vector2 &uv3,
                   const Ogre::String materialName) : d(new TrianglePrivate()) {
  // vertex positions
  d->position[0] = p1;
  d->position[1] = p2;
  d->position[2] = p3;
  // face normal
  d->normal = Ogre::Math::calculateBasicFaceNormal(p1, p2, p3);
  // texture coordinates
  d->uv[0] = uv1;
  d->uv[1] = uv2;
  d->uv[2] = uv3;
  // material name
  d->materialName = materialName;
  // precompute some values
  d->b = d->position[2] - d->position[0];
  d->c = d->position[1] - d->position[0];
  if (fabs(d->normal.x) > fabs(d->normal.y)) {
    if (fabs(d->normal.x) > fabs(d->normal.z))
      d->axis = 0;
    else
      d->axis = 2;
  } else {
    if (fabs(d->normal.y) > fabs(d->normal.z))
      d->axis = 1;
    else
      d->axis = 2;
  }
  d->uAxis = (d->axis + 1) % 3;
  d->vAxis = (d->axis + 2) % 3;
  d->inv_det = 1.0f / (d->b[d->uAxis] * d->c[d->vAxis] - d->b[d->vAxis] * d->c[d->uAxis]);
}

Triangle::~Triangle() {
  delete d;
}

const Ogre::Vector3 Triangle::position(const int i) const {
  return d->position[i];
}

const Ogre::Vector3 Triangle::normal(const Ogre::Real u, const Ogre::Real v) const {
  return d->normal;
}

const Ogre::Vector2 Triangle::texCoord(const Ogre::Real u, const Ogre::Real v) const {
  return d->uv[0] * (1 - u - v) + d->uv[1] * u + d->uv[2] * v;
}

const Ogre::String Triangle::materialName() const {
  return d->materialName;
}

const Ogre::ColourValue Triangle::getAmbientColour(const Ogre::Real u, const Ogre::Real v) const {
  // TODO: Implement
  return Ogre::ColourValue::White;
}

const Ogre::ColourValue Triangle::getDiffuseColour(const Ogre::Real u, const Ogre::Real v) const {
  // TODO: Implement
  return Ogre::ColourValue::White;
}

const Ogre::ColourValue Triangle::getSpecularColour(const Ogre::Real u, const Ogre::Real v) const {
  // TODO: Implement
  return Ogre::ColourValue::White;
}

const bool Triangle::intersects(const Ogre::Ray &ray, Ogre::Real &t, Ogre::Real &u, Ogre::Real &v) const {
  t = -(ray.getOrigin() - d->position[0]).dotProduct(d->normal) / ray.getDirection().dotProduct(d->normal);
  if (t < std::numeric_limits<float>::epsilon())
    return false;
  // calculate hit point
  Ogre::Vector3 hit = ray.getPoint(t) - d->position[0];
  // calculate u
  u = (d->b[d->uAxis] * hit[d->vAxis] - d->b[d->vAxis] * hit[d->uAxis]) * d->inv_det;
  if (u < 0)
    return false;
  // calculate v
  v = (d->c[d->vAxis] * hit[d->uAxis] - d->c[d->uAxis] * hit[d->vAxis]) * d->inv_det;
  if (v < 0)
    return false;
  // u + v should be less than or equal to 1
  if (u + v > 1)
    return false;
  // ray intersects triangle
  return true;
}

const bool Triangle::intersects(const Ogre::Ray &ray) const {
  Ogre::Real t, u, v;
  t = -(ray.getOrigin() - d->position[0]).dotProduct(d->normal) / ray.getDirection().dotProduct(d->normal);
  if (t < std::numeric_limits<float>::epsilon())
    return false;
  // calculate hit point
  Ogre::Vector3 hit = ray.getPoint(t) - d->position[0];
  // calculate u
  u = (d->b[d->uAxis] * hit[d->vAxis] - d->b[d->vAxis] * hit[d->uAxis]) * d->inv_det;
  if (u < 0)
    return false;
  // calculate v
  v = (d->c[d->vAxis] * hit[d->uAxis] - d->c[d->uAxis] * hit[d->vAxis]) * d->inv_det;
  if (v < 0)
    return false;
  // u + v should be less than or equal to 1
  if (u + v > 1)
    return false;
  // ray intersects triangle
  return true;
//  return Ogre::Math::intersects(ray, d->position[0], d->position[1], d->position[2], true, false).first;
}
