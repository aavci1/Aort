#ifndef OGREMANAGER_H
#define OGREMANAGER_H

#include <QObject>

namespace Ogre {
  class Camera;
  class Entity;
  class RenderWindow;
  class SceneManager;
}

class MainWindow;

class OgreManagerPrivate;

class OgreManager : public QObject {
  Q_OBJECT

public:
  OgreManager(MainWindow *parent);
  ~OgreManager();

  static OgreManager *instance();

  Ogre::SceneManager *sceneManager();

public slots:
  Ogre::RenderWindow *createWindow(QWidget *widget, int width, int height);
  Ogre::Camera *createCamera(QString name);
  Ogre::Entity *loadMesh(QString path);

private:
  OgreManagerPrivate *d;
};

#endif // OGREMANAGER_H
