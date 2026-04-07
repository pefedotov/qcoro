// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: MIT

#pragma once

#include "qcoroqml_export.h"

#include "qcoroqmltask.h"

namespace QCoro::Qml {

[[deprecated("Use 'import QCoro' in QML instead")]]
QCOROQML_EXPORT void registerTypes();

}
