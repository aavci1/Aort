#!/bin/sh

mkdir -p translations

translations=(tr)

for translation in ${translations[*]}
do
  lupdate -no-obsolete -no-ui-lines src/*.* -ts translations/Aort_$translation.ts
done
