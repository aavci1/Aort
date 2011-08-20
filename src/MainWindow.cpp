#include "MainWindow.h"

#include "LanguageManager.h"
#include "OgreManager.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreViewport.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), Ui::MainWindow() {
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
  // connect language action handler
  connect(actionGroupLanguages, SIGNAL(triggered(QAction*)), this, SLOT(translate(QAction*)));
  // connect action handlers
  connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
  connect(actionRender, SIGNAL(triggered()), this, SLOT(render()));
  connect(actionHelp, SIGNAL(triggered()), this, SLOT(help()));
  connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  // create ogre manager instance
  new OgreManager(this);
  // connect ogre widget signals
  connect(ogreWidget, SIGNAL(windowCreated()), this, SLOT(windowCreated()));
}

MainWindow::~MainWindow() {
}

void MainWindow::changeEvent(QEvent *e) {
  if (e->type() == QEvent::LanguageChange)
    retranslateUi(this);
}

void MainWindow::translate(QAction *action) {
  // select language
  LanguageManager::instance()->selectLanguage(action->data().toString());
  // save language settings
  QSettings().setValue("Language", action->data().toString());
}

void MainWindow::open() {
  QString path = QFileDialog::getOpenFileName(this, tr("Open File"), QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation), tr("Mesh Files (*.mesh)"));
  // return if open canceled
  if (path.isNull())
    return;
  // update window title
  setWindowTitle(QString("%1 - Aort").arg(path));
  // TODO: load the file
}

void MainWindow::render() {
  // TODO: render the scene
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
  camera->setPosition(0.0f, 250.0f, 250.0f);
  camera->lookAt(0, 0, 0);
  // create viewport
  viewport = ogreWidget->renderWindow()->addViewport(camera);
  viewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
}
