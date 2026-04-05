<!--
SPDX-FileCopyrightText: 2022 Daniel Vrátil <dvratil@kde.org>

SPDX-License-Identifier: GFDL-1.3-or-later
-->

# QDBusPendingReply

 {{ doctable("DBus", "QCoroDBusPendingReply") }}

[`QDBusPendingReply`][qdoc-qdbuspendingreply] on its own doesn't have any operation that could
be awaited asynchronously, this is usually done through a helper class called
[`QDBusPendingCallWatcher`][qdoc-qdbuspendingcallwatcher]. To simplify the API, QCoro allows to
directly `co_await` completion of the pending reply or use a wrapper class `QCoroDBusPendingReply`.
To wrap a `QDBusPendingReply` into a `QCoroDBusPendingReply`, use [`qCoro()`][qcoro-coro]:

```cpp
template<typename ... Args>
QCoroDBusPendingCall qCoro(const QDBusPendingReply<Args ...> &);
```

To await completion of the pending call without the `qCoro` wrapper, just use the pending call
in a `co_await` expression. The behavior is identical to awaiting on result of
`QCoroDBusPendingReply::waitForFinished()`.

```cpp
QDBusPendingReply<...> reply = interface.asyncCall(...);
co_await reply;
// Now the reply is finished and the result can be retrieved.
```

## `waitForFinished()`

{% include-markdown "../../../qcoro/dbus/qcorodbuspendingreply.h"
    dedent=true
    rewrite-relative-urls=false
    start="<!-- doc-waitForFinished-start -->"
    end="<!-- doc-waitForFinished-end -->" %}

## Example

```cpp
{% include "../../examples/qdbus.cpp" %}
```

[qdoc-qdbuspendingcall]: https://doc.qt.io/qt-6/qdbuspendingcall.html
[qdoc-qdbuspendingreply]: https://doc.qt.io/qt-6/qdbuspendingreply.html
[qdoc-qdbuspendingcallwatcher]: https://doc.qt.io/qt-6/qdbuspendingcallwatcher.html
[qdoc-qdbuspendingcallwatcher-finished]: https://doc.qt.io/qt-6/qdbuspendingcallwatcher.html#finished
[qdoc-qdbusabstractinterface-asyncCall]: https://doc.qt.io/qt-6/qdbusabstractinterface.html#asyncCall-1
[qcoro-coro]: ../coro/coro.md
