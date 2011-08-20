#ifndef OGRERENDERER_H
#define OGRERENDERER_H

#include <QImage>
#include <QObject>

namespace Ogre {
  class Camera;
  class SceneNode;
}

class OgreRendererPrivate;

class OgreRenderer : public QObject {
  Q_OBJECT
public:
  OgreRenderer(QObject *parent = 0);
  ~OgreRenderer();

  QImage render(Ogre::SceneNode *root, const Ogre::Camera *camera, const int width, const int height, const int maxReflection = 2);

private:
  OgreRendererPrivate *d;
};

#endif // OGRERENDERER_H
