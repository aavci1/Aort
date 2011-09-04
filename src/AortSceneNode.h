#ifndef AORTSCENENODE_H
#define AORTSCENENODE_H

#include <OGRE/OgrePrerequisites.h>

#include <float.h>

#define MAXIMUM_DEPTH (32)
#define MINIMUM_TRIANGLES_PER_LEAF (4)

namespace Aort {
  class Triangle;

  class SceneNode {
  public:
    SceneNode();
    SceneNode(const Ogre::AxisAlignedBox &aabb, std::vector<Triangle *> &triangles);
    ~SceneNode();

    const bool hit(const Ogre::Ray &ray, Triangle *&triangle, Ogre::Real &t, Ogre::Real &u, Ogre::Real &v, const Ogre::Real t_min = 0.0f, const Ogre::Real t_max = FLT_MAX) const;
    const bool hit(const Ogre::Ray &ray, const Ogre::Real t_min = 0.0f, const Ogre::Real t_max = FLT_MAX) const;

    static size_t intersectionCount;

  private:
    void split(const Ogre::AxisAlignedBox &aabb, std::vector<Triangle *> &triangles, const int depth = 0);

    const bool isLeaf() const;
    void setLeaf(const bool leaf);

    const int axis() const;
    void setAxis(const int axis);

    SceneNode *nodes() const;
    Triangle **triangles() const;

    void setPointer(const void *pointer);

  private:
    unsigned long data;
    Ogre::Real splitPosition;
  };
}

#endif // AORTSCENENODE_H
