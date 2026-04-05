// SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: MIT

#include "qcoroqmltask.h"

#include <QLoggingCategory>

#include <optional>

QT_WARNING_PUSH
QT_WARNING_DISABLE_MSVC(4458 4201)
#include <private/qjsvalue_p.h>
QT_WARNING_POP

Q_DECLARE_LOGGING_CATEGORY(qcoroqml)
Q_LOGGING_CATEGORY(qcoroqml, "qcoro.qml")

namespace QCoro {

struct QmlTaskPrivate : public QSharedData {
    std::optional<QCoro::Task<QVariant>> task;

    QmlTaskPrivate() = default;
    QmlTaskPrivate(const QmlTaskPrivate &) : QSharedData() {
        Q_UNREACHABLE();
    };
};

inline QJSEngine *getEngineForValue(const QJSValue &val) {
    return QJSValuePrivate::engine(&val)->jsEngine();
}

QmlTask::QmlTask() noexcept : d(new QmlTaskPrivate())
{
}

QmlTask::QmlTask(QCoro::Task<QVariant> &&task)
    : d(new QmlTaskPrivate())
{
    d->task = std::move(task);
}

QmlTask &QmlTask::operator=(const QmlTask &other) = default;
QmlTask::QmlTask(const QmlTask &other) = default;
QmlTask::~QmlTask() = default;

void QmlTask::then(QJSValue func) {
    if (!d->task) {
        qCWarning(qcoroqml, ".then called on a QmlTask that is not connected to any coroutine. "
                            "Make sure you don't default-construct QmlTask in your code");
        return;
    }

    if (!func.isCallable()) {
        qCWarning(qcoroqml, ".then called with an argument that is not a function. The .then call will do nothing");
        return;
    }

    d->task->then([func = std::move(func)](const QVariant &result) mutable -> void {
        auto jsval = getEngineForValue(func)->toScriptValue(result);
        func.call({jsval});
    });
}

QmlTaskListener *QmlTask::await(const QVariant &intermediateValue)
{
    QPointer listener = new QmlTaskListener();
    if (!intermediateValue.isNull()) {
        listener->setValue(QVariant(intermediateValue));
    }
    d->task->then([listener](auto &&value) {
        if (listener) {
            listener->setValue(std::move(value));
        }
    });
    return listener;
}

QVariant QmlTaskListener::value() const
{
    return m_value;
}

void QmlTaskListener::setValue(QVariant &&value)
{
    m_value = std::move(value);
    Q_EMIT valueChanged();
}

}
