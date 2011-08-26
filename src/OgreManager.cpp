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
#include <OGRE/OgreSubMesh.h>

#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>

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
  d->sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
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

Ogre::Camera *OgreManager::createCamera(const QString &name) {
  return d->sceneManager->createCamera(name.toStdString());
}


Ogre::Entity *OgreManager::loadMesh(const QString &path) {
  Ogre::String source = path.toStdString();
  // check file extension
  if (path.toLower().endsWith(".mesh")) {
    // load the mesh file content
    Ogre::DataStreamPtr stream(new Ogre::FileStreamDataStream(new std::ifstream(source.c_str())));
    // create the mesh pointer
    Ogre::MeshPtr meshPtr = Ogre::MeshManager::getSingletonPtr()->createManual(source, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    // import mesh content into the mesh pointer
    Ogre::MeshSerializer().importMesh(stream, meshPtr.getPointer());
  } else {
    // try to loading the mesh using assimp
    Assimp::Importer *importer = new Assimp::Importer();
    const aiScene *scene = importer->ReadFile(source.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_PreTransformVertices | aiProcess_TransformUVCoords | aiProcess_FlipUVs);
    if (scene->HasMaterials()) {
      // TODO: load materials
    }
    Ogre::MeshPtr meshPtr = Ogre::MeshManager::getSingletonPtr()->createManual(source, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::AxisAlignedBox aabb;
    if (scene->HasMeshes()) {
      for (int i = 0; i < scene->mNumMeshes; ++i) {
        const struct aiMesh *aimesh = scene->mMeshes[i];
        // create a submesh
        Ogre::SubMesh *submesh = meshPtr->createSubMesh();
        // create vertex buffer
        submesh->useSharedVertices = false;
        submesh->vertexData = new Ogre::VertexData();
        submesh->vertexData->vertexStart = 0;
        submesh->vertexData->vertexCount = aimesh->mNumVertices;
        // create vertex declaration
        Ogre::VertexDeclaration* declaration = submesh->vertexData->vertexDeclaration;
        unsigned short start = 0;
        size_t offset = 0;
        // get pointers to the position, normal and texture coordinates arrays
        aiVector3D *position = aimesh->mVertices;
        aiVector3D *normal = aimesh->mNormals;
        aiVector3D *texCoord = aimesh->mTextureCoords[0];
        // create position element
        if (position)
          offset += declaration->addElement(start, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
        // create normal element
        if (normal)
          offset += declaration->addElement(start, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL).getSize();
        // create texture coords element
        if (texCoord)
          offset += declaration->addElement(start, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES).getSize();
        // create vertex buffer
        Ogre::HardwareVertexBufferSharedPtr vertexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(declaration->getVertexSize(start),
            submesh->vertexData->vertexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        // get vertex buffer
        float* vertexData = static_cast<float*>(vertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        // parse vertices
        for (size_t j = 0; j < aimesh->mNumVertices; ++j) {
          // vertex position
          if (aimesh->HasPositions()) {
            *vertexData++ = position->x;
            *vertexData++ = position->y;
            *vertexData++ = position->z;
            // next vertex
            position++;
            // update bounding box
            aabb.merge(Ogre::Vector3(position->x, position->y, position->z));
          }
          // vertex normal
          if (aimesh->HasNormals()) {
            *vertexData++ = normal->x;
            *vertexData++ = normal->y;
            *vertexData++ = normal->z;
            // next vertex
            normal++;
          }
          // texture coordinates
          if (aimesh->HasTextureCoords(0)) {
            *vertexData++ = texCoord->x;
            *vertexData++ = texCoord->y;
            // next vertex
            texCoord++;
          }
        }
        vertexBuffer->unlock();
        // bind the buffer
        submesh->vertexData->vertexBufferBinding->setBinding(start, vertexBuffer);
        // create index data
        submesh->indexData->indexStart = 0;
        submesh->indexData->indexCount = aimesh->mNumFaces * 3;
        // get pointer to the face data
        aiFace *f = aimesh->mFaces;
        // index buffer
        submesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()->createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT,
                                          submesh->indexData->indexCount,
                                          Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        Ogre::uint16* idata = static_cast<Ogre::uint16*>(submesh->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        // faces
        for (size_t j = 0; j < aimesh->mNumFaces; ++j) {
          *idata++ = f->mIndices[0];
          *idata++ = f->mIndices[1];
          *idata++ = f->mIndices[2];
          // next face
          f++;
        }
        submesh->indexData->indexBuffer->unlock();
        // TODO: assign material
      }
    }
    meshPtr->_setBounds(aabb);
    meshPtr->_setBoundingSphereRadius((aabb.getMaximum() - aabb.getMinimum()).length() * 0.5f);
    // clean up
    delete importer;
  }
  // create and return an entity from the mesh
  return d->sceneManager->createEntity(source);
}
