#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"

namespace Ogre {
  class Camera;
  class Viewport;
}

class MainWindow : public QMainWindow, public Ui::MainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void changeEvent(QEvent *e);

private slots:
  void open();
  void translate(QAction *action);
  void render();
  void help();
  void about();
  void windowCreated();

private:
  Ogre::Camera *camera;
  Ogre::Viewport *viewport;
};

#endif // MAINWINDOW_H
