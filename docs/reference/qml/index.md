<!--
SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
SPDX-License-Identifier: GFDL-1.3-or-later
-->

# QML Module

The QCoro `QML` module contains coroutine-friendly wrappers for [QtQml][qtdoc-qml] classes.

## CMake Usage

```cmake
find_package(QCoro6 COMPONENTS Qml)
...
target_link_libraries(my-target QCoro::Qml)
```

## QMake Usage

```
QT += QCoroQml
```


[qtdoc-qml]: https://doc.qt.io/qt-6/qml-index.html
