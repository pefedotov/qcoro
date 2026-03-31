#!/bin/bash

set -e

qt_version="$1"
qt_modules="$2"
qt_archives="$3"

if [[ -n "${qt_archives}" ]]; then
    opt_archives="--archives ${qt_archives}"
fi
if [[ -n "${qt_modules}" ]]; then
    opt_modules="--modules ${qt_modules}"
fi

if [[ $(echo "${qt_version}" | cut -d'.' -f1) == "5" ]]; then
    arch="gcc_64"
else
    arch="linux_gcc_64"
fi

echo "Installing Qt ${qt_version}"
echo "Modules: ${qt_modules}"
echo "Archives: ${qt_archives}"
aqt install-qt -O /opt/qt linux desktop "${qt_version}" "${arch}" ${opt_archives} ${opt_modules}
echo "Done."
