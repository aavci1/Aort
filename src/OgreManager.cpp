#include "OgreManager.h"

#include "MainWindow.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreDataStream.h>
#include <OGRE/OgreMeshManager.h>
#include <OGRE/OgreMeshSerializer.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

static OgreManager *_instance = 0;

class OgreManagerPrivate {
public:
  OgreManagerPrivate() : root(0), sceneManager(0) {
  }

  ~OgreManagerPrivate() {
    delete root;
  }

  void updateViews() {
    for (int i = 0; i < windows.size(); ++i)
      windows.at(i)->update(true);
  }

  Ogre::Root *root;
  Ogre::SceneManager *sceneManager;
  QList<Ogre::RenderWindow *> windows;
};

OgreManager::OgreManager(MainWindow *parent) : QObject(parent), d(new OgreManagerPrivate()) {
  _instance = this;
  // initialize OGRE
  d->root = new Ogre::Root("", "", "");
  // load plugins
#ifdef Q_WS_X11
  d->root->loadPlugin("/usr/lib/OGRE/RenderSystem_GL.so");
#endif
#ifdef USE_OPENGL
  d->root->loadPlugin("RenderSystem_GL.dll");
#endif
#ifdef USE_DIRECTX
  d->root->loadPlugin("RenderSystem_Direct3D9.dll");
#endif
  // select the rendersystem
  d->root->setRenderSystem(d->root->getAvailableRenderers().at(0));
  // initialise root object
  d->root->initialise(false);
  // create 1x1 top level window
  createWindow(parent, 1, 1);
  // add resource locations
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/materials", "FileSystem");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/meshes", "FileSystem");
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation("media/textures", "FileSystem");
  // load resources
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
  // create the scene manager
  d->sceneManager = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC);
  // use stencil shadows
  d->sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
  // set up ambient light
  d->sceneManager->setAmbientLight(Ogre::ColourValue(0.1f, 0.1f, 0.1f));
}

OgreManager::~OgreManager() {
  delete d;
}

OgreManager *OgreManager::instance() {
  return _instance;
}

Ogre::SceneManager *OgreManager::sceneManager() {
  return d->sceneManager;
}

Ogre::RenderWindow *OgreManager::createWindow(QWidget *widget, int width, int height) {
  // create render window
  Ogre::NameValuePairList options;
#ifdef Q_WS_X11
  const QX11Info &info = widget->x11Info();
  options["externalWindowHandle"] = QString("%1:%2:%3")
                                    .arg((unsigned long)info.display())
                                    .arg((unsigned int)info.screen())
                                    .arg((unsigned long)widget->winId()).toStdString();
#else
  options["externalWindowHandle"] = QString("%1").arg((unsigned long)widget->winId()).toStdString();
#endif
  options["FSAA"] = "4";
  // create a new window
  Ogre::RenderWindow *window = d->root->createRenderWindow(QString("OgreWindow-%1").arg(d->windows.size()).toStdString(), width, height, false, &options);
  // save window
  d->windows.append(window);
  // return created window
  return window;
}

Ogre::Camera *OgreManager::createCamera(QString name) {
  return d->sceneManager->createCamera(name.toStdString());
}

Ogre::Entity *OgreManager::loadMesh(QString path) {
  Ogre::String source = path.toStdString();
  // load the mesh file content
  Ogre::DataStreamPtr stream(new Ogre::FileStreamDataStream(new std::ifstream(source.c_str())));
  // create the mesh pointer
  Ogre::MeshPtr meshPtr = Ogre::MeshManager::getSingleton().createManual(source, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  // import mesh content into the mesh pointer
  Ogre::MeshSerializer().importMesh(stream, meshPtr.getPointer());
  // create and return an entity from the mesh
  return d->sceneManager->createEntity(source);
}
