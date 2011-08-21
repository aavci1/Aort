#ifndef KDTREE_H
#define KDTREE_H

#include <float.h>

#define MAXIMUM_DEPTH (64)
#define MINIMUM_TRIANGLES_PER_LEAF (2)

class Triangle;

namespace Ogre {
  class AxisAlignedBox;
  class Ray;
}

class KdTreeNode {
public:
  KdTreeNode();
  KdTreeNode(const Ogre::AxisAlignedBox &aabb, const void *pointer);

  const bool intersects(const Ogre::Ray &ray, const float t_min = 0.0f, const float t_max = FLT_MAX) const;

private:
  void split(const Ogre::AxisAlignedBox &aabb, const int depth);

  const bool isLeaf() const;
  void setLeaf(const bool leaf);

  const int axis() const;
  void setAxis(const int axis);

  KdTreeNode *nodes() const;
  Triangle **triangles() const;

  void setPointer(const void *pointer);

private:
  unsigned long mData;
  float mSplitPosition;
};

#endif // KDTREE_H
