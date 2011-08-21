#include "Triangle.h"

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

const Ogre::Vector2 Triangle::textureCoord(const Ogre::Real u, const Ogre::Real v) const {
  return d->uv[0] * (1 - u - v) + d->uv[1] * u + d->uv[2] * v;
}

const Ogre::String Triangle::materialName() const {
  return d->materialName;
}

const int mod[5] = {0, 1, 2, 0, 1};
// TODO: speed up by precomputing as much as you can
const bool Triangle::intersects(const Ogre::Ray &ray, Ogre::Real &t, Ogre::Real &u, Ogre::Real &v) const {
  Ogre::Vector3 b = d->position[2] - d->position[0];
  Ogre::Vector3 c = d->position[1] - d->position[0];
  t = -(ray.getOrigin() - d->position[0]).dotProduct(d->normal) / ray.getDirection().dotProduct(d->normal);
  if (t <= 0)
    return false;
  int k;
  if (fabs(d->normal.x) > fabs(d->normal.y)) {
    if (fabs(d->normal.x) > fabs(d->normal.z))
      k = 0;
    else
      k = 2;
  } else {
    if (fabs(d->normal.y) > fabs(d->normal.z))
      k = 1;
    else
      k = 2;
  }
  int _u = mod[k + 1];
  int _v = mod[k + 2];
  Ogre::Vector3 H(0, 0, 0);
  H[_u] = (ray.getOrigin()[_u] + t * ray.getDirection()[_u]) - d->position[0][_u];
  H[_v] = (ray.getOrigin()[_v] + t * ray.getDirection()[_v]) - d->position[0][_v];
  float denom = b[_u] * c[_v] - b[_v] * c[_u];
  u = (b[_u] * H[_v] - b[_v] * H[_u]) / denom;
  if (u < 0)
    return false;
  v = (c[_v] * H[_u] - c[_u] * H[_v]) / denom;
  if (v < 0)
    return false;
  if (v + u > 1.0)
    return false;
  if (ray.getDirection().dotProduct(d->normal) > 0)
    return false;
  return true;
}

const bool Triangle::intersects(const Ogre::Ray &ray) const {
  Ogre::Real t, u, v;
  Ogre::Vector3 b = d->position[2] - d->position[0];
  Ogre::Vector3 c = d->position[1] - d->position[0];
  t = -(ray.getOrigin() - d->position[0]).dotProduct(d->normal) / ray.getDirection().dotProduct(d->normal);
  if (t <= 0)
    return false;
  int k;
  if (fabs(d->normal.x) > fabs(d->normal.y)) {
    if (fabs(d->normal.x) > fabs(d->normal.z))
      k = 0;
    else
      k = 2;
  } else {
    if (fabs(d->normal.y) > fabs(d->normal.z))
      k = 1;
    else
      k = 2;
  }
  int _u = mod[k + 1];
  int _v = mod[k + 2];
  Ogre::Vector3 H(0, 0, 0);
  H[_u] = (ray.getOrigin()[_u] + t * ray.getDirection()[_u]) - d->position[0][_u];
  H[_v] = (ray.getOrigin()[_v] + t * ray.getDirection()[_v]) - d->position[0][_v];
  float denom = b[_u] * c[_v] - b[_v] * c[_u];
  u = (b[_u] * H[_v] - b[_v] * H[_u]) / denom;
  if (u < 0)
    return false;
  v = (c[_v] * H[_u] - c[_u] * H[_v]) / denom;
  if (v < 0)
    return false;
  if (v + u > 1.0)
    return false;
  if (ray.getDirection().dotProduct(d->normal) > 0)
    return false;
  return true;
//  return Ogre::Math::intersects(ray, d->position[0], d->position[1], d->position[2], true, false).first;
}
