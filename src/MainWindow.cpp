#include "MainWindow.h"

#include "AortRenderer.h"
#include "OgreManager.h"
#include "TranslationManager.h"

#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QWheelEvent>
#include <QTimer>
#include <QToolButton>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreMeshManager.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreViewport.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), Ui::MainWindow(), mTranslationManager(new TranslationManager()), objectNode(0), camera(0), viewport(0) {
  setupUi(this);
  // set window title
  setWindowTitle(tr("Untitled - Aort"));
  // create the language menu actions
  QActionGroup *actionGroupLanguages = new QActionGroup(this);
  actionGroupLanguages->setExclusive(true);
  QStringList codes = mTranslationManager->translations().keys();
  QStringList names = mTranslationManager->translations().values();
  for (int i = 0; i < codes.count(); ++i) {
    QAction *action = new QAction(names.at(i), actionGroupLanguages);
    action->setCheckable(true);
    action->setData(codes.at(i));
    action->setChecked(mTranslationManager->currentTranslation() == codes.at(i));
  }
  // create the language menu
  QMenu *languageMenu = new QMenu(this);
  languageMenu->addActions(actionGroupLanguages->actions());
  actionLanguage->setMenu(languageMenu);
  // make language menu shown instantly
  for (int i = 0; i < actionLanguage->associatedWidgets().size(); ++i) {
    QToolButton *toolbutton= qobject_cast<QToolButton *>(actionLanguage->associatedWidgets().at(i));
    if (toolbutton)
      toolbutton->setPopupMode(QToolButton::InstantPopup);
  }
  // connect language action handler
  connect(actionGroupLanguages, SIGNAL(triggered(QAction*)), this, SLOT(translate(QAction*)));
  // main window action handlers
  connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
  connect(actionRender, SIGNAL(triggered()), this, SLOT(render()));
  connect(actionHelp, SIGNAL(triggered()), this, SLOT(help()));
  connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  // ogre widget handlers
  connect(ogreWidget, SIGNAL(windowCreated()), this, SLOT(windowCreated()));
  connect(ogreWidget, SIGNAL(mousePressed(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));
  connect(ogreWidget, SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
  connect(ogreWidget, SIGNAL(wheelMoved(QWheelEvent*)), this, SLOT(wheelMoved(QWheelEvent*)));
  // initialize ogre manager
  new OgreManager(this);
}

MainWindow::~MainWindow() {
  delete mTranslationManager;
}

void MainWindow::changeEvent(QEvent *e) {
  if (e->type() == QEvent::LanguageChange)
    retranslateUi(this);
}

void MainWindow::mousePressed(QMouseEvent *e) {
  panStart = e->pos();
}

void MainWindow::mouseMoved(QMouseEvent *e) {
  if (e->buttons() == Qt::RightButton) {
    // update camera position
    camera->moveRelative(Ogre::Vector3(panStart.x() - e->pos().x(), e->pos().y() - panStart.y(), 0));
    // update pan start
    panStart = e->pos();
    // update view
    ogreWidget->update();
  } else if (e->buttons() == Qt::MiddleButton) {
    // rotate camera
    camera->yaw(Ogre::Degree(-0.1f * (e->x() - mousePosition.x())));
    camera->pitch(Ogre::Degree(-0.1f * (e->y() - mousePosition.y())));
    // update view
    ogreWidget->update();
  }
  // update mouse position
  mousePosition = e->pos();
}

void MainWindow::wheelMoved(QWheelEvent *e) {
  float altitude = camera->getPosition().y;
  camera->moveRelative(Ogre::Vector3(0, 0, -0.4f * e->delta()));
  camera->setPosition(camera->getPosition().x, altitude, camera->getPosition().z);
  // update view
  ogreWidget->update();
}

void MainWindow::open() {
  QString path = QFileDialog::getOpenFileName(this, tr("Open File"), QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation), tr("All Supported Formats (%1)").arg(OgreManager::instance()->supportedFormats()));
  // return if open canceled
  if (path.isNull())
    return;
  // update window title
  setWindowTitle(QString("%1 - Aort").arg(path));
  // delete previous entities
  objectNode->removeAndDestroyAllChildren();
  // load and attach the new entity
  Ogre::Entity *object = OgreManager::instance()->loadMesh(path);
  objectNode->createChildSceneNode()->attachObject(object);
  // put object just on the ground
  object->getParentSceneNode()->translate(0, -(object->getWorldBoundingBox(true).getMinimum().y + object->getWorldBoundingBox(true).getSize().y * Ogre::MeshManager::getSingletonPtr()->getBoundsPaddingFactor()), 0.0f);
}

void MainWindow::translate(QAction *action) {
  mTranslationManager->loadTranslation(action->data().toString());
}

void MainWindow::render() {
  // TODO: make image size configurable
  int width = 800;
  int height = 545;
  int fsaa = 1;
  // create renderer
  Aort::Renderer *renderer = new Aort::Renderer();
  // do preprocess
  qDebug() << "Preprocessing finished in" << renderer->preprocess(OgreManager::instance()->sceneManager()->getRootSceneNode()) << "ms";
  // create buffer
  uchar *buffer = new uchar[width * fsaa * height * fsaa * 4];
  // do render
  qDebug() << "Rendering finished in" << renderer->render(camera, width * fsaa, height * fsaa, buffer) << "ms";
  // clean up
  delete renderer;
  // construct default file name
  QString fileName = QString("render-%1-%2x%3.png").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmm")).arg(width).arg(height);
  // get path from the user
  QString path = QFileDialog::getSaveFileName(this, tr("Save File"), QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/" + fileName, tr("Image Files (*.png *.jpg *.jpeg)"));
  // save image
  if (!path.isNull())
    QImage(buffer, width * fsaa, height * fsaa, QImage::Format_ARGB32_Premultiplied).scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save(path);
  // clean up
  delete buffer;

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
  // set camera position and orientation
  camera->setPosition(Ogre::Vector3(0, 180, 0));
  camera->setOrientation(Ogre::Quaternion(1, 0, 0, 0));
  // create viewport
  viewport = ogreWidget->renderWindow()->addViewport(camera);
  viewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
}
