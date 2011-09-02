#include "AortSceneNode.h"

#include "AortTriangle.h"

#include <OGRE/OgreAxisAlignedBox.h>
#include <OGRE/OgreRay.h>

namespace Aort {
  enum SplitPointType  {
    Maximum = 0,
    Minimum = 2
  };

  class SplitPoint {
  public:
    SplitPoint(Ogre::Real position, Triangle *triangle, SplitPointType type) : position(position), triangle(triangle), type(type) {
    }

    Ogre::Real position;
    Triangle *triangle;
    SplitPointType type;
  };

  // custom comparator for sweep events
  bool splitPointCompare(const SplitPoint &s1, const SplitPoint &s2) {
    if (s1.position < s2.position)
      return true;
    if (s1.position > s2.position)
      return false;
    return s1.type < s2.type;
  }

  SceneNode::SceneNode() : data(6), splitPosition(0) {
  }

  SceneNode::SceneNode(const Ogre::AxisAlignedBox &aabb, std::vector<Triangle *> &triangles) : data(6), splitPosition(0) {
    split(aabb, triangles);
  }

  SceneNode::~SceneNode() {
    if (isLeaf())
      delete[] triangles();
    else
      delete[] nodes();
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

  void SceneNode::split(const Ogre::AxisAlignedBox &aabb, std::vector<Triangle *> &triangles, const int depth) {
    // if maximum depth or minimum triangle count has been reached, dont split
    if (depth >= MAXIMUM_DEPTH || triangles.size() <= MINIMUM_TRIANGLES_PER_LEAF) {
      // create triangles array
      Triangle** t = new Triangle*[triangles.size() + 1];
      t[triangles.size()] = 0;
      // copy triangle pointers
      for (size_t i = 0; i < triangles.size(); ++i)
        t[i] = triangles.at(i);
      // save triangle pointer
      setPointer(t);
      // return
      return;
    }
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
    Ogre::Real minimum = aabb.getMinimum()[splitAxis];
    Ogre::Real maximum = aabb.getMaximum()[splitAxis];
    std::vector<SplitPoint> splitPoints;
    // generate split points
    for (size_t i = 0; i < triangles.size(); ++i) {
      // clip triangle to the bounding box
      Ogre::Real min = std::max(triangles.at(i)->getMinimum()[splitAxis], minimum);
      Ogre::Real max = std::min(triangles.at(i)->getMaximum()[splitAxis], maximum);
      // push split points
      splitPoints.push_back(SplitPoint(min, triangles.at(i), Minimum));
      splitPoints.push_back(SplitPoint(max, triangles.at(i), Maximum));
    }
    // sort events
    std::sort(splitPoints.begin(), splitPoints.end(), splitPointCompare);
    // find the best split point
    Ogre::Real splitCost = FLT_MAX;
    size_t left = 0, right = triangles.size();
    for (size_t i = 0; i < splitPoints.size(); ++i) {
      Ogre::Real position = splitPoints.at(i).position;
      // count points at this point
      size_t e = 0, s = 0;
      while ((i < splitPoints.size()) && (splitPoints.at(i).position == position)) {
        if (splitPoints.at(i).type == Maximum)
          e++;
        else
          s++;
        i++;
      }
      // update triangle counts
      right -= e;
      left += s;
      // avoid re-creating the same node
      if ((position == minimum && left == 0) || (position == maximum && right == 0))
        continue;
      // calculate cost of splitting
      Ogre::Real cost = ((position - minimum) * (a + b) + a * b) * left + ((maximum - position) * (a + b) + a * b) * right;
      // update optimal point if needed
      if (cost < splitCost) {
        splitCost = cost;
        splitPosition = position;
      }
    }
    if (splitCost >= (size[splitAxis] * (a + b) + a * b) * triangles.size()) {
      // create triangles array
      Triangle** t = new Triangle*[triangles.size() + 1];
      t[triangles.size()] = 0;
      // copy triangle pointers
      for (size_t i = 0; i < triangles.size(); ++i)
        t[i] = triangles.at(i);
      // save triangle pointer
      setPointer(t);
      // return
      return;
    }
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
    std::vector<Triangle *> leftTriangles, rightTriangles;
    for (size_t i = 0; i < triangles.size(); ++i) {
      if (triangles.at(i)->getMinimum()[splitAxis] <= splitPosition)
        leftTriangles.push_back(triangles[i]);
      if (triangles.at(i)->getMaximum()[splitAxis] > splitPosition)
        rightTriangles.push_back(triangles[i]);
    }
    // calculate left bounding box
    Ogre::AxisAlignedBox lbb = aabb;
    lbb.getMaximum()[splitAxis] = splitPosition;
    // calculate right bounding box
    Ogre::AxisAlignedBox rbb = aabb;
    rbb.getMinimum()[splitAxis] = splitPosition;
    // split left node
    leftNode->split(lbb, leftTriangles, depth + 1);
    // split right node
    rightNode->split(rbb, rightTriangles, depth + 1);
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
