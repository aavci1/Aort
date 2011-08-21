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
  // constructor
  KdTreeNode() : mData(6), mSplitPosition(0) { }
  KdTreeNode(void *pointer) : mData(6), mSplitPosition(0) {
    setPointer(pointer);
  }
  // functions
  void subdivide(Ogre::AxisAlignedBox aabb, int depth);
  bool anyHit(const Ogre::Ray &ray, const float t_min = 0.0f, const float t_max = FLT_MAX);
private:
  bool closestHit(const Ogre::Ray &ray, Triangle **triangle, float *t, float *u, float *v, float t_min = 0.0f, float t_max = FLT_MAX);

  bool isLeaf() {
    return ((mData & 4) > 0);
  }
  void setLeaf(bool leaf) {
    mData = (leaf) ? (mData | 4) : (mData & 0xfffffffb);
  }

  void axis(int axis) {
    mData = (mData & 0xfffffffc) + axis;
  }
  int getAxis() {
    return mData & 3;
  }

  // sets the pointer to the either triangles or the child nodes
  void setPointer(void *pointer) {
    mData = (unsigned long)pointer + (mData & 7);
  }
  // returns the pointer to the either triangles or the child nodes
  void *pointer() {
    return (void*)(mData & 0xfffffff8);
  }
  // members
  unsigned long mData;
  float mSplitPosition;
};

#endif // KDTREE_H
