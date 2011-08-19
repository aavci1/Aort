#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QHash>
#include <QString>

class LanguageManagerPrivate;

class LanguageManager : QObject {
  Q_OBJECT

public:
  LanguageManager(QObject *parent = 0);
  ~LanguageManager();

  static LanguageManager *instance();

  const QHash<QString, QString> &languages() const;
  const QString &currentLanguage() const;

public slots:
  void loadTranslation(const QString &code, const QString &name);
  void selectLanguage(const QString &code);

private:
  LanguageManagerPrivate *d;
};

#endif // LANGUAGEMANAGER_H
