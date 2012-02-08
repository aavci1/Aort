#include "TranslationManager.h"

#include <QApplication>
#include <QLocale>
#include <QSettings>

TranslationManager::TranslationManager() {
  // populate the language hash
  mLanguages.insert("en", QString::fromUtf8("English"));
  mLanguages.insert("tr", QString::fromUtf8("Türkçe"));
  // load the translations
  for (int i = 0; i < mLanguages.keys().count(); ++i) {
    // skip source language
    if (mLanguages.keys().at(i) == "en")
      continue;
    // create the translator
    QTranslator *translator = new QTranslator();
    translator->load("translations/Aort_" + mLanguages.keys().at(i));
    // save translator
    mTranslators.insert(mLanguages.keys().at(i), translator);
  }
  // update language
  QSettings settings;
  mCurrentLanguage = settings.value("Language", "").toString();
  // get system language
  QString systemLanguage = QLocale::system().name().section("_", 0, 0, QString::SectionSkipEmpty);;
  // if no language is set and system language is supported, use the system language
  if (mCurrentLanguage == "" && mLanguages.contains(systemLanguage))
    mCurrentLanguage = systemLanguage;
  // load translation if needed
  if (mCurrentLanguage != "en")
    qApp->installTranslator(mTranslators[mCurrentLanguage]);
}

TranslationManager::~TranslationManager() {
  qDeleteAll(mTranslators.values());
}

QHash<QString, QString> TranslationManager::translations() {
  return mLanguages;
}

QString TranslationManager::currentTranslation() {
  return mCurrentLanguage;
}

void TranslationManager::loadTranslation(QString code) {
  // uninstall current translator
  if (mCurrentLanguage != "en")
    qApp->removeTranslator(mTranslators[mCurrentLanguage]);
  // update current language
  mCurrentLanguage = code;
  // install the translator if needed
  if (mCurrentLanguage != "en")
    qApp->installTranslator(mTranslators[mCurrentLanguage]);
  // update settings
  QSettings settings;
  settings.setValue("Language", mCurrentLanguage);
  settings.sync();
}
