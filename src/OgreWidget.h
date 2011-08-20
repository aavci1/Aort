#ifndef OGREWIDGET_H
#define OGREWIDGET_H

#include <QWidget>

class QPaintEngine;

namespace Ogre {
  class RenderWindow;
}

class OgreWidgetPrivate;

class OgreWidget : public QWidget {
  Q_OBJECT
public:
  OgreWidget(QWidget *parent = 0);
  ~OgreWidget();

public:
  Ogre::RenderWindow *renderWindow() const;

protected:
  QPaintEngine *paintEngine() const;

  void paintEvent(QPaintEvent *e);
  void resizeEvent(QResizeEvent *e);

  void keyPressEvent(QKeyEvent *event);
  void keyReleaseEvent(QKeyEvent *event);

  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

  void wheelEvent(QWheelEvent *event);

  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void dropEvent(QDropEvent *event);

signals:
  void windowCreated();

  void keyPressed(QKeyEvent *event);
  void keyReleased(QKeyEvent *event);

  void mouseMoved(QMouseEvent *event);
  void mousePressed(QMouseEvent *event);
  void mouseReleased(QMouseEvent *event);

  void wheelMoved(QWheelEvent *event);

  void dragEntered(QDragEnterEvent *event);
  void dragMoved(QDragMoveEvent *event);
  void dragLeft(QDragLeaveEvent *event);
  void dropped(QDropEvent *event);

private:
  OgreWidgetPrivate *d;
};

#endif
