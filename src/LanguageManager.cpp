#include "LanguageManager.h"

#include <QApplication>
#include <QTranslator>

static LanguageManager *_instance = 0;

class LanguageManagerPrivate {
public:
  LanguageManagerPrivate() : mCurrentLanguage("en") {
    // populate the language hash
    mLanguages.insert("en", QString::fromUtf8("English"));
  }

  ~LanguageManagerPrivate() {
    qDeleteAll(mTranslators.values());
  }

  QString mCurrentLanguage;
  QHash<QString, QString> mLanguages;
  QHash<QString, QTranslator *> mTranslators;
};

LanguageManager::LanguageManager(QObject *parent) : QObject(parent), d(new LanguageManagerPrivate()) {
  _instance = this;
}

LanguageManager::~LanguageManager() {
  delete d;
}

LanguageManager *LanguageManager::instance() {
  if (!_instance)
    _instance = new LanguageManager();
  return _instance;
}

const QHash<QString, QString> &LanguageManager::languages() const {
  return d->mLanguages;
}

const QString &LanguageManager::currentLanguage() const {
  return d->mCurrentLanguage;
}

void LanguageManager::loadTranslation(const QString &code, const QString &name) {
  QTranslator *translator = new QTranslator();
  translator->load(QString("Aort_%1").arg(code), QString("translations"));
  // save translator for further use
  d->mTranslators.insert(code, translator);
  // save language name for further use
  d->mLanguages.insert(code, name);
}

void LanguageManager::selectLanguage(const QString &code) {
  // uninstall current translator
  if (d->mCurrentLanguage != "en")
    qApp->removeTranslator(d->mTranslators[d->mCurrentLanguage]);
  // update current language
  d->mCurrentLanguage = code;
  // install the translator if needed
  if (d->mCurrentLanguage != "en")
    qApp->installTranslator(d->mTranslators[d->mCurrentLanguage]);
}
