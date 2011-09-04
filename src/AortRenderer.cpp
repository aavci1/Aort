#include "AortRenderer.h"

#include "AortLight.h"
#include "AortMaterial.h"
#include "AortMeshParser.h"
#include "AortSceneNode.h"
#include "AortTriangle.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreLight.h>
#include <OGRE/OgreMovableObject.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>

#include <limits>

#ifndef NO_OMP
#include <omp.h>
#endif // !NO_OMP

#define EPSILON 0.001f

namespace Aort {
  class RendererPrivate {
  public:
    RendererPrivate() : ambientColour(0.0f, 0.0f, 0.0f), backgroundColour(0.0f, 0.0f, 0.0f), maxDepth(0), rootNode(0), rayCount(0) {
    }

    ~RendererPrivate() {
    }

    void traverse(Ogre::SceneNode *root) {
      for (int i = 0; i < root->numAttachedObjects(); ++i) {
        Ogre::MovableObject *object = root->getAttachedObject(i);
        // skip unvisible objects
        if (!object->isVisible())
          continue;
        // get object if it is an entity or light
        if (object->getMovableType() == "Entity")
          entities.push_back(static_cast<Ogre::Entity *>(object));
        else if (object->getMovableType() == "Light")
          lights.push_back(processLight(static_cast<Ogre::Light *>(object)));
      }
      // traverse child nodes
      for (int i = 0; i < root->numChildren(); ++i)
        traverse(static_cast<Ogre::SceneNode *>(root->getChild(i)));
    }

    Light *processLight(Ogre::Light *light) {
      Light *l = new Light();
      // copy light properties
      l->setDiffuseColour(light->getDiffuseColour());
      l->setSpecularColour(light->getSpecularColour());
      l->setPosition(light->getDerivedPosition());
      l->setDirection(light->getDirection());
      // set light type
      if (light->getType() == Ogre::Light::LT_POINT)
        l->setType(Aort::LT_POINT);
      else if (light->getType() == Ogre::Light::LT_DIRECTIONAL)
        l->setType(Aort::LT_AREA);
      // return light
      return l;
    }

    void buildTree() {
      Ogre::AxisAlignedBox aabb(Ogre::Vector3(0, 0, 0), Ogre::Vector3(0, 0, 0));
      // extract triangles from all meshes
      for (int i = 0; i < entities.size(); ++i) {
        // extend aabb
        aabb.merge(entities.at(i)->getWorldBoundingBox(true));
        // extract mesh info
        MeshParser *meshParser = new MeshParser(entities.at(i));
        // merge mesh triangles
        triangles.insert(triangles.end(), meshParser->triangles().begin(), meshParser->triangles().end());
        // delete mesh parser instance
        delete meshParser;
      }
      // build the scene tree
      rootNode = new SceneNode(aabb, triangles);
    }

    Ogre::ColourValue traceRay(const Ogre::Ray &ray, int depth = 0) {
      // trace ray using the kd-tree
      Triangle *triangle = 0;
      Ogre::Real t = FLT_MAX, u = 0, v = 0;
      // increase ray count
      rayCount++;
      // if nothing hit, return background color
      if (!rootNode->hit(ray, triangle, t, u, v))
        return backgroundColour;
      // final colour
      Ogre::ColourValue finalColour(0.0f, 0.0f, 0.0f);
      // calculate view vector
      Ogre::Vector3 V = ray.getDirection();
      // calculate hit point
      Ogre::Vector3 P = ray.getPoint(t - EPSILON);
      // calculate triangle normal
      Ogre::Vector3 N = triangle->normal(u, v);
      // add ambient lighting
      finalColour += ambientColour * triangle->getMaterial()->getAmbient();
      // get diffuse colour
      Ogre::ColourValue diffuseColour = triangle->getMaterial()->getColourAt(triangle->texCoord(u, v));
      // get specular colour
      Ogre::ColourValue specularColour = triangle->getMaterial()->getSpecular();
      for (int i = 0; i < lights.size(); ++i) {
        // calculate light vector
        Ogre::Vector3 L = lights.at(i)->getPosition() - P;
        Ogre::Real length = L.normalise();
        // calculate illumination
        Ogre::Real illumination = 0.0f;
        if (lights.at(i)->getType() == Aort::LT_POINT)
          illumination = calculateIllumination(P, L, length);
        else if (lights.at(i)->getType() == LT_AREA)
          illumination = calculateIllumination(P, lights.at(i));
        // if not completely unlit
        if (illumination > std::numeric_limits<float>::epsilon()) {
          // add diffuse
          finalColour += illumination * calculateDiffuse(N, L) * diffuseColour * lights.at(i)->getDiffuseColour();
          // add specular
          finalColour += illumination * calculateSpecular(V, N, L) * specularColour * lights.at(i)->getSpecularColour();
        }
      }
      // add reflections
      if (triangle->getMaterial()->getReflectivity() > std::numeric_limits<float>::epsilon() && depth < maxDepth)
        finalColour += triangle->getMaterial()->getReflectivity() * calculateReflection(P, V, N, depth) * diffuseColour;
      // set full opacity
      finalColour.r = Ogre::Math::Clamp(finalColour.r, 0.0f, 1.0f);
      finalColour.g = Ogre::Math::Clamp(finalColour.g, 0.0f, 1.0f);
      finalColour.b = Ogre::Math::Clamp(finalColour.b, 0.0f, 1.0f);
      finalColour.a = 1.0f;
      // return final color
      return finalColour;
    }

    Ogre::Real calculateIllumination(const Ogre::Vector3 &P, const Ogre::Vector3 &L, Ogre::Real length) {
      // increase ray count
      rayCount++;
      // check for occluders
      if (!rootNode->hit(Ogre::Ray(P, L), EPSILON, length))
        return 1.0f;
      return 0.0f;
    }

    Ogre::Real calculateIllumination(const Ogre::Vector3 &P, Light *light) {
      Ogre::Real illumination = 0;
      // get some control points on the light
      const Ogre::Vector3 *points = light->getPoints();
      // check for null
      if (!points)
        return 0.0f;
      // check if visible from current point
      for (int i = 0; i < 16; ++i) {
        Ogre::Vector3 L = points[i] - P;
        Ogre::Real length = L.normalise();
        // increase ray count
        rayCount++;
        // check for occluders
        if (!rootNode->hit(Ogre::Ray(P, L), EPSILON, length))
          illumination += 1.0f / 16.0f;
      }
      return illumination;
    }

    Ogre::Real calculateDiffuse(const Ogre::Vector3 &N, const Ogre::Vector3 &L) {
      float dot = N.dotProduct(L);
      if (dot > 0.0f)
        return dot;
      return 0;
    }

    Ogre::Real calculateSpecular(const Ogre::Vector3 &V, const Ogre::Vector3 &N, const Ogre::Vector3 &L) {
      // TODO: make specular configurable
      Ogre::Vector3 R = L - 2.0f * N.dotProduct(L) * N;
      float dot = V.dotProduct(R);
      if (dot > 0)
        return  dot / (50 - 50 * dot + dot);
      return 0;
    }

    Ogre::ColourValue calculateReflection(const Ogre::Vector3 &P, const Ogre::Vector3 &V, const Ogre::Vector3 &N, int depth = 0) {
      // calculate reflection vector
      Ogre::Vector3 R = V - 2.0f * N.dotProduct(V) * N;
      // TODO: Implement diffuse (scattered) reflections
      return traceRay(Ogre::Ray(P + R * EPSILON, R), depth + 1);
    }

    Ogre::ColourValue ambientColour;
    Ogre::ColourValue backgroundColour;
    std::vector<Ogre::Entity *> entities;
    std::vector<Triangle *> triangles;
    std::vector<Light *> lights;
    size_t maxDepth;
    SceneNode *rootNode;
    size_t rayCount;
  };

  Renderer::Renderer() : d(new RendererPrivate()) {
  }

  Renderer::~Renderer() {
    delete d;
  }

  unsigned char *Renderer::render(Ogre::SceneNode *root, const Ogre::Camera *camera, const int width, const int height) {
    // log message
    Ogre::LogManager::getSingletonPtr()->logMessage("Building...");
    // extract entities and lights
    d->traverse(root);
    // build tree
    d->buildTree();
    // log message
    Ogre::LogManager::getSingletonPtr()->logMessage("Rendering...");
    // set ambient colour
    d->ambientColour = Ogre::ColourValue(0.0f, 0.0f, 0.0f);
    d->backgroundColour = Ogre::ColourValue(0.0f, 0.0f, 0.0f);
    d->maxDepth = 3;
    // create empty image
    unsigned char *result = new unsigned char[height * width * 4];
    // precalculate 1/width and 1/height
    Ogre::Real inverseWidth = 1.0f / width;
    Ogre::Real inverseHeight = 1.0f / height;
    size_t rowsCompleted = 0;
#ifndef NO_OMP
    #pragma omp parallel for
#endif // !NO_OMP
    // start rendering
    for (size_t y = 0; y < height; ++y) {
      unsigned char *scanline = result + y * width * 4;
      for (size_t x = 0; x < width; ++x) {
        // create camera to viewport ray
        // and make sure that rays are not parallel to any axis
        Ogre::Ray ray = camera->getCameraToViewportRay(x * inverseWidth + std::numeric_limits<float>::epsilon(),
                        y * inverseHeight + std::numeric_limits<float>::epsilon());
        // trace the ray
        Ogre::ColourValue colour = d->traceRay(ray);
        // update image
        scanline[x * 4 + 0] = colour.r * 255;
        scanline[x * 4 + 1] = colour.g * 255;
        scanline[x * 4 + 2] = colour.b * 255;
        scanline[x * 4 + 3] = colour.a * 255;
      }
      // increase row count
      rowsCompleted++;
      // log message
      Ogre::LogManager::getSingletonPtr()->logMessage("Progress: " + Ogre::StringConverter::toString(int(rowsCompleted * inverseHeight * 100), 3) + "%");
    }
    Ogre::LogManager::getSingletonPtr()->logMessage("Finished.");
    Ogre::LogManager::getSingletonPtr()->logMessage("Number of triangles: " + Ogre::StringConverter::toString(d->triangles.size()));
    Ogre::LogManager::getSingletonPtr()->logMessage("Number of rays: " + Ogre::StringConverter::toString(d->rayCount));
    // delete rootNode
    delete d->rootNode;
    d->rootNode = 0;
    // reset ray count
    d->rayCount = 0;
    // delete triangles
    for (int i = 0; i < d->triangles.size(); ++i)
      delete d->triangles.at(i);
    d->triangles.clear();
    // delete lights
    for (int i = 0; i < d->lights.size(); ++i)
      delete d->lights.at(i);
    d->lights.clear();
    // clean up entities
    d->entities.clear();
    // return result
    return result;
  }
}
