#include "AortSceneNode.h"

#include "AortTriangle.h"

#include <OGRE/OgreAxisAlignedBox.h>
#include <OGRE/OgreRay.h>

namespace Aort {
  SceneNode::SceneNode() : data(6), splitPosition(0) {
  }

  SceneNode::SceneNode(const Ogre::AxisAlignedBox &aabb, const void *pointer, size_t triangleCount) : data(6), splitPosition(0) {
    setPointer(pointer);
    split(aabb, triangleCount);
  }

  SceneNode::~SceneNode() {
    if (isLeaf()) {
      Triangle **triangles = this->triangles();
//    // delete triangles
//    for (int i = 0; triangles[i] != 0; ++i)
//      delete triangles[i];
      // delete triangle array
      delete[] triangles;
    } else {
      delete[] nodes();
    }
  }

  const bool SceneNode::hit(const Ogre::Ray &ray, Triangle *&triangle, Ogre::Real &t, Ogre::Real &u, Ogre::Real &v, const Ogre::Real t_min, const Ogre::Real t_max) const {
    // if leaf, check triangle list for intersection
    if (isLeaf()) {
      t = FLT_MAX;
      for (Triangle **it = triangles(); *it != 0; ++it) {
        Ogre::Real _t = FLT_MAX, _u = 0, _v = 0;
        if ((*it)->intersects(ray, _t, _u, _v) && _t >= t_min && _t <= t_max && _t < t) {
          triangle = *it;
          t = _t;
          u = _u;
          v = _v;
        }
      }
      // return result
      return (t != FLT_MAX);
    }
    SceneNode *near = (ray.getDirection()[axis()] > 0) ? nodes() + 0 : nodes() + 1;
    SceneNode *far = (ray.getDirection()[axis()] > 0) ? nodes() + 1 : nodes() + 0;
    // calculate distance to the split plane
    Ogre::Real t_split = (splitPosition - ray.getOrigin()[axis()]) / ray.getDirection()[axis()];
    // only intersects near node
    if (t_min < t_split && t_max < t_split)
      return near->hit(ray, triangle, t, u, v, t_min, t_max);
    // only intersects far node
    if (t_min > t_split && t_max > t_split)
      return far->hit(ray, triangle, t, u, v, t_min, t_max);
    // intersects both
    if (near->hit(ray, triangle, t, u, v, t_min, t_split))
      return true;
    if (far->hit(ray, triangle, t, u, v, t_split, t_max))
      return true;
    // no hit, return false
    return false;
  }

  const bool SceneNode::hit(const Ogre::Ray &ray, const Ogre::Real t_min, const Ogre::Real t_max) const {
    // if leaf, check triangle list for intersection
    if (isLeaf()) {
      for (Triangle **it = triangles(); *it != 0; ++it) {
        Ogre::Real _t = FLT_MAX, _u = 0, _v = 0;
        // if intersects and intersection is between the t_min and t_max
        if ((*it)->intersects(ray, _t, _u, _v) && _t >= t_min && _t <= t_max)
          return true;
      }
      // no hit, return false
      return false;
    }
    SceneNode *near = (ray.getDirection()[axis()] > 0) ? nodes() + 0 : nodes() + 1;
    SceneNode *far = (ray.getDirection()[axis()] > 0) ? nodes() + 1 : nodes() + 0;
    // calculate distance to the split plane
    Ogre::Real t_split = (splitPosition - ray.getOrigin()[axis()]) / ray.getDirection()[axis()];
    // only intersects near node
    if (t_min < t_split && t_max < t_split)
      return near->hit(ray, t_min, t_max);
    // only intersects far node
    if (t_min > t_split && t_max > t_split)
      return far->hit(ray, t_min, t_max);
    // intersects both
    if (near->hit(ray, t_min, t_split))
      return true;
    if (far->hit(ray, t_split, t_max))
      return true;
    // no hit, return false
    return false;
  }

  void SceneNode::split(const Ogre::AxisAlignedBox &aabb, size_t triangleCount, const int depth) {
    // if maximum depth or minimum triangle count has been reached, dont split
    if (depth >= MAXIMUM_DEPTH || triangleCount <= MINIMUM_TRIANGLES_PER_LEAF)
      return;
    Triangle **triangles = this->triangles();
    // get bounding box size
    Ogre::Vector3 size = aabb.getSize();
    // assume first axis is the longest one
    int splitAxis = 0;
    // update axis, if second axis is longer
    if (size[1] > size[splitAxis])
      splitAxis = 1;
    // update axis, if third axis is longer
    if (size[2] > size[splitAxis])
      splitAxis = 2;
    Ogre::Real a = size[(splitAxis + 1) % 3];
    Ogre::Real b = size[(splitAxis + 2) % 3];
    Ogre::Real boxMinimum = aabb.getMinimum()[splitAxis];
    Ogre::Real boxMaximum = aabb.getMaximum()[splitAxis];
    std::vector<Ogre::Real> splitCandidates;
    Ogre::Real *minimums = new Ogre::Real[triangleCount];
    Ogre::Real *maximums = new Ogre::Real[triangleCount];
    Ogre::Real *minimums2 = new Ogre::Real[triangleCount];
    Ogre::Real *maximums2 = new Ogre::Real[triangleCount];
    for (size_t i = 0; i < triangleCount; ++i) {
      Ogre::Real minimum = std::min(std::min(triangles[i]->position(0)[splitAxis], triangles[i]->position(1)[splitAxis]),
                                    triangles[i]->position(2)[splitAxis]);
      Ogre::Real maximum = std::max(std::max(triangles[i]->position(0)[splitAxis], triangles[i]->position(1)[splitAxis]),
                                    triangles[i]->position(2)[splitAxis]);
      minimums[i] = minimum;
      maximums[i] = maximum;
      minimums2[i] = minimum;
      maximums2[i] = maximum;
      if (minimum > boxMinimum)
        splitCandidates.push_back(minimum);
      if (maximum < boxMaximum)
        splitCandidates.push_back(maximum);
    }
    // sort split candidates
    std::sort(splitCandidates.begin(), splitCandidates.end());
    std::sort(minimums, minimums + triangleCount);
    std::sort(maximums, maximums + triangleCount);
    size_t leftCount = 0;
    size_t rightCount = 0;
    Ogre::Real lowestCost = FLT_MAX;
    for (size_t i = 0; i < splitCandidates.size(); ++i) {
      size_t left = 0;
      while (left < triangleCount && minimums[left] <= splitCandidates.at(i))
        left++;
      size_t right = 0;
      while (right < triangleCount && maximums[triangleCount - right - 1] > splitCandidates.at(i))
        right++;
      // calculate surface areas of left and right boxes
      Ogre::Real SAL = (splitCandidates.at(i) - boxMinimum) * (a + b) + a * b;
      Ogre::Real SAR = (boxMaximum - splitCandidates.at(i)) * (a + b) + a * b;
      // calculate splitting cost
      Ogre::Real cost = SAL * left + SAR * right;
      if (cost < lowestCost) {
        lowestCost = cost;
        splitPosition = splitCandidates.at(i);
        leftCount = left;
        rightCount = right;
      }
    }
    if (lowestCost > (size[splitAxis] * (a + b) + a * b) * triangleCount)
      return;
    // split
    setLeaf(false);
    setAxis(splitAxis);
    // point the pointer to the nodes
    setPointer(new SceneNode[2]);
    // create left node
    SceneNode *leftNode = nodes() + 0;
    // create node
    SceneNode *rightNode = nodes() + 1;
    // add triangles
    Triangle **leftTriangles = new Triangle*[leftCount + 1];
    Triangle **rightTriangles = new Triangle*[rightCount + 1];
    size_t l = 0, r = 0;
    for (size_t j = 0; j < triangleCount; ++j) {
      if (l < leftCount && minimums2[j] <= splitPosition)
        leftTriangles[l++] = triangles[j];
      if (r < rightCount && maximums2[j] > splitPosition)
        rightTriangles[r++] = triangles[j];
    }
    leftTriangles[leftCount] = 0;
    rightTriangles[rightCount] = 0;
    // remove triangles from this node
    delete[] triangles;
    delete[] minimums;
    delete[] maximums;
    delete[] minimums2;
    delete[] maximums2;
    // set left triangles
    leftNode->setPointer(leftTriangles);
    // set right triangles
    rightNode->setPointer(rightTriangles);
    // calculate left bounding box
    Ogre::AxisAlignedBox lbb = aabb;
    lbb.getMaximum()[splitAxis] = splitPosition;
    // calculate right bounding box
    Ogre::AxisAlignedBox rbb = aabb;
    rbb.getMinimum()[splitAxis] = splitPosition;
    // split left node
    leftNode->split(lbb, leftCount, depth + 1);
    // split right node
    rightNode->split(rbb, rightCount, depth + 1);
  }

  const bool SceneNode::isLeaf() const {
    return ((data & 4) > 0);
  }

  void SceneNode::setLeaf(const bool leaf) {
    data = (leaf) ? (data | 4) : (data & 0xfffffffb);
  }

  const int SceneNode::axis() const {
    return data & 3;
  }

  void SceneNode::setAxis(const int axis) {
    data = (data & 0xfffffffc) + axis;
  }

  SceneNode *SceneNode::nodes() const {
    return (SceneNode *)(data & 0xfffffff8);
  }

  Triangle **SceneNode::triangles() const {
    return (Triangle **)(data & 0xfffffff8);
  }

  void SceneNode::setPointer(const void *pointer) {
    data = (unsigned long)pointer + (data & 7);
  }
}
