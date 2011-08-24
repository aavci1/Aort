#include "MainWindow.h"

#include "LanguageManager.h"
#include "OgreManager.h"
#include "OgreRenderer.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QWheelEvent>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreViewport.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), Ui::MainWindow(), cameraNode(0), objectNode(0), camera(0), viewport(0) {
  setupUi(this);
  // set window title
  setWindowTitle(tr("Untitled - Aort"));
  // load languages
  LanguageManager::instance()->loadTranslation(QString("tr"), QString::fromUtf8("Türkçe"));
  // load language settings
  QString language = QSettings().value("Language", "").toString();
  // try to load saved language, if cant try to load system language
  if (LanguageManager::instance()->languages().contains(language))
    LanguageManager::instance()->selectLanguage(language);
  else if (LanguageManager::instance()->languages().contains(QLocale::system().name().section("_", 0, 0, QString::SectionSkipEmpty)))
    LanguageManager::instance()->selectLanguage(QLocale::system().name().section("_", 0, 0, QString::SectionSkipEmpty));
  // create the languages menu
  QActionGroup *actionGroupLanguages = new QActionGroup(this);
  actionGroupLanguages->setExclusive(true);
  QStringList codes = LanguageManager::instance()->languages().keys();
  QStringList names = LanguageManager::instance()->languages().values();
  for (int i = 0; i < codes.count(); ++i) {
    QAction *action = new QAction(names.at(i), actionGroupLanguages);
    action->setCheckable(true);
    action->setData(codes.at(i));
    action->setChecked(LanguageManager::instance()->currentLanguage() == codes.at(i));
  }
  // create the language menu
  QMenu *languageMenu = new QMenu(this);
  languageMenu->addActions(actionGroupLanguages->actions());
  actionLanguage->setMenu(languageMenu);
  // language action handler
  connect(actionGroupLanguages, SIGNAL(triggered(QAction*)), this, SLOT(translate(QAction*)));
  // main window action handlers
  connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
  connect(actionRender, SIGNAL(triggered()), this, SLOT(render()));
  connect(actionHelp, SIGNAL(triggered()), this, SLOT(help()));
  connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  // ogre widget handlers
  connect(ogreWidget, SIGNAL(windowCreated()), this, SLOT(windowCreated()));
  connect(ogreWidget, SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
  connect(ogreWidget, SIGNAL(wheelMoved(QWheelEvent*)), this, SLOT(wheelMoved(QWheelEvent*)));
  // initialize ogre manager
  new OgreManager(this);
}

MainWindow::~MainWindow() {
}

void MainWindow::changeEvent(QEvent *e) {
  if (e->type() == QEvent::LanguageChange)
    retranslateUi(this);
}

void MainWindow::mouseMoved(QMouseEvent *event) {
  if (event->buttons() == Qt::LeftButton) {
    // rotate camera
    cameraNode->getParentSceneNode()->yaw(Ogre::Degree(-0.25f * (event->x() - mousePosition.x())));
    cameraNode->getParentSceneNode()->pitch(Ogre::Degree(-0.25f * (event->y() - mousePosition.y())));
    // update view
    ogreWidget->update();
  } else if (event->buttons() == Qt::RightButton) {
    // update camera position
    cameraNode->translate(Ogre::Vector3(mousePosition.x() - event->pos().x(), event->pos().y() - mousePosition.y(), 0));
    // update view
    ogreWidget->update();
  }
  // update mouse position
  mousePosition = event->pos();
}

void MainWindow::wheelMoved(QWheelEvent *event) {
  cameraNode->translate(0.0f, 0.0f, -0.05f * event->delta());
  // update view
  ogreWidget->update();
}

void MainWindow::open() {
  QString path = QFileDialog::getOpenFileName(this, tr("Open File"), QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation), tr("Mesh Files (*.mesh)"));
  // return if open canceled
  if (path.isNull())
    return;
  // update window title
  setWindowTitle(QString("%1 - Aort").arg(path));
  // delete previous entities
  objectNode->removeAndDestroyAllChildren();
  // load and attach the new entity
  objectNode->createChildSceneNode()->attachObject(OgreManager::instance()->loadMesh(path));
}

void MainWindow::translate(QAction *action) {
  // select language
  LanguageManager::instance()->selectLanguage(action->data().toString());
  // save language settings
  QSettings().setValue("Language", action->data().toString());
}

void MainWindow::render() {
  // TODO: make image size configurable
  int width = 800;
  int height = 545;
  // render the scene
  QImage render = OgreRenderer().render(OgreManager::instance()->sceneManager()->getRootSceneNode(), camera, width, height);
  // save image file
  QString fileName = QString("render-%1-%2x%3.png").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmm")).arg(width).arg(height);
  QString path = QFileDialog::getSaveFileName(this, tr("Save File"), QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/" + fileName, tr("Image Files (*.png *.jpg *.jpeg)"));
  // return if open canceled
  if (path.isNull())
    return;
  // save the image
  render.save(path);
}

void MainWindow::help() {
  // TODO: open help document
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About %1").arg(QCoreApplication::applicationName()),
                     tr("<b>%1 %2</b><br><br>Copyright (c) 2011 <a href='%4'>%3</a>. All rights reserved.<br><br>"
                        "The program is provided AS IS with NO WARRANTY OF ANY KIND,<br>"
                        "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND<br>"
                        "FITNESS FOR A PARTICULAR PURPOSE.")
                     .arg(QCoreApplication::applicationName())
                     .arg(QCoreApplication::applicationVersion())
                     .arg(QCoreApplication::organizationName())
                     .arg(QCoreApplication::organizationDomain()));
}

void MainWindow::windowCreated() {
  // create camera
  camera = OgreManager::instance()->createCamera("Camera");
  camera->setNearClipDistance(10.0f);
  camera->setFarClipDistance(10000.0f);
  camera->setAutoAspectRatio(true);
  // create viewport
  viewport = ogreWidget->renderWindow()->addViewport(camera);
  viewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
  // create camera node
  cameraNode = OgreManager::instance()->sceneManager()->getRootSceneNode()->createChildSceneNode()->createChildSceneNode(Ogre::Vector3(0.0f, 0.0f, 250.0f));
  cameraNode->lookAt(Ogre::Vector3(0.0f, 0.0f, 0.0f), Ogre::SceneNode::TS_WORLD);
  // attach camera to the node
  cameraNode->attachObject(camera);
  // create object node
  objectNode = OgreManager::instance()->sceneManager()->getRootSceneNode()->createChildSceneNode();
  // create a light
  Ogre::Light *light = OgreManager::instance()->sceneManager()->createLight();
  light->setType(Ogre::Light::LT_POINT);
  light->setDiffuseColour(0.5f, 0.5f, 0.5f);
  light->setSpecularColour(1.0f, 1.0f, 1.0f);
  // attach the light to the scene
  OgreManager::instance()->sceneManager()->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0.0f, 250.0f, 0.0f))->attachObject(light);
  // create a head light
  Ogre::Light *headLight = OgreManager::instance()->sceneManager()->createLight();
  headLight->setType(Ogre::Light::LT_POINT);
  headLight->setDiffuseColour(0.5f, 0.5f, 0.5f);
  headLight->setSpecularColour(1.0f, 1.0f, 1.0f);
  // attach light to the camera node
  cameraNode->attachObject(headLight);
}
