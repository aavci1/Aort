#ifndef AORTRENDERER_H
#define AORTRENDERER_H

#include <OGRE/OgrePrerequisites.h>

namespace Aort {
  class RendererPrivate;

  class Renderer {
  public:
    Renderer();
    ~Renderer();

    void preprocess(Ogre::SceneNode *root);
    unsigned char *render(const Ogre::Camera *camera, const int width, const int height);

  private:
    RendererPrivate *d;
  };
}

#endif // AORTRENDERER_H
