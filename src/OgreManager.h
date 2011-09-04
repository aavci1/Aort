#ifndef OGREMANAGER_H
#define OGREMANAGER_H

#include <QObject>

#include <OGRE/OgrePrerequisites.h>

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
  Ogre::Camera *createCamera(const QString &name);

  const QString supportedFormats() const;
  Ogre::Entity *loadMesh(const QString &path);

private:
  OgreManagerPrivate *d;
};

#endif // OGREMANAGER_H
