#include "KdTree.h"

#include "Triangle.h"

#include <OGRE/OgreAxisAlignedBox.h>
#include <OGRE/OgreRay.h>

KdTreeNode::KdTreeNode() : mData(6), mSplitPosition(0) {
}

KdTreeNode::KdTreeNode(const Ogre::AxisAlignedBox &aabb, const void *pointer) : mData(6), mSplitPosition(0) {
  setPointer(pointer);
  split(aabb, 0);
}

const bool KdTreeNode::intersects(const Ogre::Ray &ray, const float t_min, const float t_max) const {
  // if leaf, check triangle list for intersection
  if (isLeaf()) {
    for (Triangle **it = triangles(); *it != 0; ++it)
      if ((*it)->intersects(ray))
        return true;
    // no hit, return false
    return false;
  }
  KdTreeNode *near = (ray.getDirection()[axis()] > 0) ? nodes() + 0 : nodes() + 1;
  KdTreeNode *far = (ray.getDirection()[axis()] > 0) ? nodes() + 1 : nodes() + 0;
  float t_split = (mSplitPosition - ray.getOrigin()[axis()]) / ray.getDirection()[axis()];
  if (t_min < t_split && t_max < t_split) {
    if (near->intersects(ray, t_min, t_max))
      return true;
    // no hit, return false
    return false;
  }
  if (t_min > t_split && t_max > t_split) {
    if (far->intersects(ray, t_min, t_max))
      return true;
    // no hit, return false
    return false;
  }
  if (near->intersects(ray, t_min, t_split))
    return true;
  if (far->intersects(ray, t_split, t_max))
    return true;
  // no hit, return false
  return false;
}

void KdTreeNode::split(const Ogre::AxisAlignedBox &aabb, const int depth) {
  Ogre::Vector3 size = aabb.getSize();
  // subdivision will use the longest axis of the bounding box
  if ((size.x >= size.y) && (size.x >= size.z)) {
    setAxis(0);
  }  else if ((size.y >= size.x) && (size.y >= size.z)) {
    setAxis(1);
  } else {
    setAxis(2);
  }
  int mSplitAxis = axis();
  float a = size[(mSplitAxis + 1) % 3];
  float b = size[(mSplitAxis + 2) % 3];
  float boxMinimum = aabb.getMinimum()[mSplitAxis];
  float boxMaximum = aabb.getMaximum()[mSplitAxis];
  std::vector<float> splitCandidates;
  unsigned int triangleCount;
  Triangle **mTriangles = triangles();
  for (triangleCount = 0; mTriangles[triangleCount] != 0; triangleCount++);
  float *minimums = new float[triangleCount];
  float *maximums = new float[triangleCount];
  float *minimums2 = new float[triangleCount];
  float *maximums2 = new float[triangleCount];
  for (unsigned int i = 0; i < triangleCount; ++i) {
    float minimum = std::min(std::min(mTriangles[i]->position(0)[mSplitAxis], mTriangles[i]->position(1)[mSplitAxis]),
                             mTriangles[i]->position(2)[mSplitAxis]);
    float maximum = std::max(std::max(mTriangles[i]->position(0)[mSplitAxis], mTriangles[i]->position(1)[mSplitAxis]),
                             mTriangles[i]->position(2)[mSplitAxis]);
    minimums[i] = minimum;
    maximums[i] = maximum;
    minimums2[i] = minimum;
    maximums2[i] = maximum;
    if (minimum > boxMinimum) {
      splitCandidates.push_back(minimum);
    }
    if (maximum < boxMaximum) {
      splitCandidates.push_back(maximum);
    }
  }
  // sort split candidates
  std::sort(splitCandidates.begin(), splitCandidates.end());
  std::sort(minimums, minimums + triangleCount);
  std::sort(maximums, maximums + triangleCount);
  unsigned int leftCount = 0;
  unsigned int rightCount = 0;
  float lowestCost = FLT_MAX;
  unsigned int left = 0;
  unsigned int right = 0;
  for (std::vector<float>::iterator it = splitCandidates.begin(); it != splitCandidates.end(); ++it) {
    while (left < triangleCount && minimums[left] <= (*it)) {
      left++;
    }
    while (right < triangleCount && maximums[right] <= (*it)) {
      right++;
    }
    // calculate surface areas of left and right boxes
    float SAL = ((*it) - boxMinimum) * (a + b) + a * b;
    float SAR = (boxMaximum - (*it)) * (a + b) + a * b;
    // calculate splitting cost
    float cost = SAL * left + SAR * (triangleCount - right);
    if (cost < lowestCost) {
      lowestCost = cost;
      mSplitPosition = (*it);
      leftCount = left;
      rightCount = (triangleCount - right);
    }
  }
  if (lowestCost > (size[mSplitAxis] * (a + b) + a * b) * triangleCount)
    return;
  // this is not a leaf anymore
  setLeaf(false);
  // triangles pointer
  Triangle **triangles = mTriangles;
  // create nodes
  KdTreeNode *mNodes = new KdTreeNode[2];
  // point the pointer to the nodes
  setPointer(mNodes);
  // create left node
  KdTreeNode *mLeftNode = mNodes;
  // create node
  KdTreeNode *mRightNode = mNodes + 1;
  // add triangles
  Triangle **leftTriangles = new Triangle*[leftCount + 1];
  Triangle **rightTriangles = new Triangle*[rightCount + 1];
  unsigned int l = 0, r = 0;
  for (unsigned int j = 0; j < triangleCount; ++j) {
    if (minimums2[j] <= mSplitPosition) {
      leftTriangles[l] = triangles[j];
      l++;
    }
    if (maximums2[j] > mSplitPosition) {
      rightTriangles[r] = triangles[j];
      r++;
    }
  }
  leftTriangles[l] = 0;
  mLeftNode->setPointer(leftTriangles);
  rightTriangles[r] = 0;
  mRightNode->setPointer(rightTriangles);
  // set bounding box
  Ogre::AxisAlignedBox lbb = aabb;
  if (mSplitAxis == 0) {
    lbb.setMaximumX(mSplitPosition);
  } else if (mSplitAxis == 1) {
    lbb.setMaximumY(mSplitPosition);
  } else {
    lbb.setMaximumZ(mSplitPosition);
  }
  if (depth < MAXIMUM_DEPTH && leftCount > MINIMUM_TRIANGLES_PER_LEAF) {
    // subdivide left node
    mLeftNode->split(lbb, depth + 1);
  }
  // set bounding box
  Ogre::AxisAlignedBox rbb = aabb;
  if (mSplitAxis == 0) {
    rbb.setMinimumX(mSplitPosition);
  } else if (mSplitAxis == 1) {
    rbb.setMinimumY(mSplitPosition);
  } else {
    rbb.setMinimumZ(mSplitPosition);
  }
  if (depth < MAXIMUM_DEPTH && rightCount > MINIMUM_TRIANGLES_PER_LEAF) {
    // subdivide node
    mRightNode->split(rbb, depth + 1);
  }
  // remove triangles from this node
  delete[] triangles;
  delete[] minimums;
  delete[] maximums;
  delete[] minimums2;
  delete[] maximums2;
}

const bool KdTreeNode::isLeaf() const {
  return ((mData & 4) > 0);
}

void KdTreeNode::setLeaf(const bool leaf) {
  mData = (leaf) ? (mData | 4) : (mData & 0xfffffffb);
}

const int KdTreeNode::axis() const {
  return mData & 3;
}

void KdTreeNode::setAxis(const int axis) {
  mData = (mData & 0xfffffffc) + axis;
}

KdTreeNode *KdTreeNode::nodes() const {
  return (KdTreeNode *)(mData & 0xfffffff8);
}

Triangle **KdTreeNode::triangles() const {
  return (Triangle **)(mData & 0xfffffff8);
}

void KdTreeNode::setPointer(const void *pointer) {
  mData = (unsigned long)pointer + (mData & 7);
}
