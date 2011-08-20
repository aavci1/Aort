#include "OgreWidget.h"

#include "OgreManager.h"

#include <QPaintEngine>

#include <OGRE/OgreRenderWindow.h>

class OgreWidgetPrivate {
public:
  OgreWidgetPrivate() : window(0) {
  }

  ~OgreWidgetPrivate() {
  }

  Ogre::RenderWindow *window;
};

OgreWidget::OgreWidget(QWidget *parent) : QWidget(parent), d(new OgreWidgetPrivate()) {
  // enable painting on screen
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_PaintOnScreen);
  setAttribute(Qt::WA_OpaquePaintEvent);
  // enable mouse tracking
  setMouseTracking(true);
  // set focus policy
  setFocusPolicy(Qt::WheelFocus);
  // accept drops
  setAcceptDrops(true);
}

OgreWidget::~OgreWidget() {
  delete d;
}

Ogre::RenderWindow *OgreWidget::renderWindow() const {
  return d->window;
}

QPaintEngine *OgreWidget::paintEngine() const {
  // Return a null paint engine to disable painting by backing store to prevent flicker
  // http://qt.nokia.com/developer/task-tracker/index_html?method=entry&id=128698
  return 0;
}

void OgreWidget::paintEvent(QPaintEvent */*e*/) {
  // create the render window if not created yet
  if (!d->window) {
    d->window = OgreManager::instance()->createWindow(this, width(), height());
    // emit signal
    emit windowCreated();
  }
  // update content
  d->window->update();
}

void OgreWidget::resizeEvent(QResizeEvent */*e*/) {
  if (!d->window)
    return;
#ifdef Q_WS_X11
  d->window->resize(width(), height());
#endif
  d->window->windowMovedOrResized();
}

void OgreWidget::keyPressEvent(QKeyEvent *event) {
  emit keyPressed(event);
}

void OgreWidget::keyReleaseEvent(QKeyEvent *event) {
  emit keyReleased(event);
}

void OgreWidget::mousePressEvent(QMouseEvent *event) {
  emit mousePressed(event);
}

void OgreWidget::mouseReleaseEvent(QMouseEvent *event) {
  emit mouseReleased(event);
}

void OgreWidget::mouseMoveEvent(QMouseEvent *event) {
  emit mouseMoved(event);
}

void OgreWidget::wheelEvent(QWheelEvent *event) {
  emit wheelMoved(event);
}

void OgreWidget::dragEnterEvent(QDragEnterEvent *event) {
  emit dragEntered(event);
}

void OgreWidget::dragMoveEvent(QDragMoveEvent *event) {
  emit dragMoved(event);
}

void OgreWidget::dragLeaveEvent(QDragLeaveEvent *event) {
  emit dragLeft(event);
}

void OgreWidget::dropEvent(QDropEvent *event) {
  emit dropped(event);
}
