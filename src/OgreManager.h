#ifndef OGREMANAGER_H
#define OGREMANAGER_H

#include <QObject>

namespace Ogre {
  class Camera;
  class RenderWindow;
}

class MainWindow;

class OgreManagerPrivate;

class OgreManager : public QObject {
  Q_OBJECT

public:
  OgreManager(MainWindow *parent);
  ~OgreManager();

  static OgreManager *instance();

public slots:
  Ogre::RenderWindow *createWindow(QWidget *widget, int width, int height);
  Ogre::Camera *createCamera(QString name);

private:
  OgreManagerPrivate *d;
};

#endif // OGREMANAGER_H
