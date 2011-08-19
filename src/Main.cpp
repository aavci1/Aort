#include <QApplication>
#include <QLocale>

#include "MainWindow.h"

#if defined(Q_WS_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR argv, INT argc) {
  QApplication app(argc, &argv);
#else
int main(int argc, char **argv) {
  QApplication app(argc, argv);
#endif
  // set company and product info
  QCoreApplication::setOrganizationName("Swiff Software Solutions");
  QCoreApplication::setOrganizationDomain("www.swiffsoftware.com");
  QCoreApplication::setApplicationName("Aort");
  QCoreApplication::setApplicationVersion("2011");
  // set default locale to english
  QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
  // create main window
  MainWindow main;
  // show main window in maximized state
  main.showMaximized();
  // start appliation event loop
  return app.exec();
}
