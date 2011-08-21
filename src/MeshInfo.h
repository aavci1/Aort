#ifndef MESHINFO_H
#define MESHINFO_H

namespace Ogre {
  class Entity;
}

class MeshInfoPrivate;
class Triangle;

class MeshInfo {
public:
  MeshInfo(const Ogre::Entity *entity);
  ~MeshInfo();

  Triangle **triangles();

private:
  MeshInfoPrivate *d;
};

#endif // MESHINFO_H
