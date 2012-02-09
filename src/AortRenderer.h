#ifndef AORTRENDERER_H
#define AORTRENDERER_H

#include <QObject>

namespace Ogre {
  class Camera;
  class SceneNode;
}

namespace Aort {
  class RendererPrivate;

  class Renderer {
  public:
    Renderer();
    ~Renderer();

    int preprocess(Ogre::SceneNode *root);
    int render(const Ogre::Camera *camera, const int width, const int height, uchar *buffer);

  private:
    RendererPrivate *d;
  };
}

#endif // AORTRENDERER_H
