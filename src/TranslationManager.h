#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QHash>
#include <QString>
#include <QTranslator>

class TranslationManager {
public:
  explicit TranslationManager();
  ~TranslationManager();

  QHash<QString, QString> translations();
  QString currentTranslation();
  void loadTranslation(QString code);

private:
  QString mCurrentLanguage;
  QHash<QString, QString> mLanguages;
  QHash<QString, QTranslator *> mTranslators;
};

#endif // TRANSLATIONMANAGER_H
