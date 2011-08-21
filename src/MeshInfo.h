#ifndef MESHINFO_H
#define MESHINFO_H

#include <stddef.h>

namespace Ogre {
  class Entity;
}

class MeshInfoPrivate;
class Triangle;

class MeshInfo {
public:
  MeshInfo(const Ogre::Entity *entity);
  ~MeshInfo();

  const size_t triangleCount() const;
  Triangle **triangles() const;

private:
  MeshInfoPrivate *d;
};

#endif // MESHINFO_H
