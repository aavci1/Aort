#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"

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
  void help();
  void about();
};

#endif // MAINWINDOW_H
