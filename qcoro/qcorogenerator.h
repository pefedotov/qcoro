// SPDX-FileCopyrightText: 2022 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <variant>
#include <exception>

#include "coroutine.h"

#include <QDebug>

namespace QCoro {

template<typename T>
class Generator;

namespace detail {

/**
 * @brief Promise type for generator coroutine.
 *
 * GeneratorPromise is automatically constructed by the compiler
 * (from Generator<T>::promise_type) inside the generator coroutine.
 *
 * The generator coroutine is suspended on start (it won't produce any value
 * until asked for).
 **/
template<typename T>
class GeneratorPromise {
public:
    /**
     * Constructs the Generator<T> object returned from the generator coroutine
     * to the caller.
     **/
    Generator<T> get_return_object();

    /**
     * Indicates that the generator should suspend on start and
     * only generate the first value when asked for.
     **/
    std::suspend_always initial_suspend() { return {}; }
    /**
     * Indicates that the generator coroutine should suspend when
     * it reaches the end (or returns), rather then destroyed.
     *
     * The generator coroutine is destroyed only when the Generator<T>
     * object is destroyed.
     **/
    std::suspend_always final_suspend() noexcept {
        mValue = std::monostate{};
        return {};
    }

    /**
     * Called automatically when the generator coroutine throws an exception.
     *
     * The thrown exception is stored in the promise and will be re-thrown in the
     * caller (see QCoro::detail::GeneratorIterator<T>::operator*()).
     **/
    void unhandled_exception() {
        mValue = std::current_exception();
    }

    /**
     * Stores the current value produced (`co_yield`ed) by the generator coroutine.
     *
     * The value is stored and in the promise and the generator coroutine
     * is suspended.
     **/
    template<typename From>
    requires std::is_convertible_v<From, T>
    std::suspend_always yield_value(From &&from) {
        mValue.template emplace<T>(std::forward<From>(from));
        return {};
    }

    /**
     * The generator coroutine itself must always be `void`.
     **/
    void return_void() {}

    /**
     * Returns the exception stored in the promise type (if any).
     **/
    std::exception_ptr exception() const {
        return std::holds_alternative<std::exception_ptr>(mValue)
            ? std::get<std::exception_ptr>(mValue)
            : std::exception_ptr{};
    }

    /**
     * Returns the current value stored in the promise type.
     **/
    T &value() {
        return std::get<T>(mValue);
    }

    /**
     * Whether the generator coroutine has finished or not.
     **/
    bool finished() const {
        return std::holds_alternative<std::monostate>(mValue);
    }

private:
    std::variant<std::monostate, T, std::exception_ptr> mValue;
};

/**
 * @brief Iterator to loop over values produced by the generator coroutine.
 *
 * Dereferencing the iterator will return a value produced by the generator coroutine.
 * If the generator coroutine has thrown an exception, dereferencing the iterator will
 * re-throw the exception.
 *
 * Incrementing the iterator will resume the generator coroutine, letting it run until
 * it yields next value, or finishes. If the generator coroutine finishes, the iterator
 * will become invalid and will be equal to Generator<T>::end().
 *
 * The iterator can only be obtained from Generator<T>::begin() and Generator<T>::end()
 * methods.
 */
template<typename T>
class GeneratorIterator {
    using promise_type = GeneratorPromise<T>;
public:
    using iterator_category = std::input_iterator_tag;
    // Not sure what type should be used for difference_type as we don't
    // allow calculating difference between two iterators.
    using difference_type = std::ptrdiff_t;
    using value_type = std::remove_reference_t<T>;
    using reference = std::add_lvalue_reference_t<T>;
    using pointer = std::add_pointer_t<value_type>;

    /**
     * @brief Resumes the generator coroutine until it yields new value or finishes.
     *
     * Returns an iterator holding the next value produced by the generator coroutine
     * or an invalid iterator, indicating the generator coroutine has finishes.
     **/
    GeneratorIterator operator++() noexcept {
        if (!mGeneratorCoroutine) {
            return *this;
        }

        mGeneratorCoroutine.resume(); // generate next value
        if (mGeneratorCoroutine.promise().finished()) {
            mGeneratorCoroutine = nullptr;
        }

        return *this;
    }

    /**
     * @brief Returns value produced by the generator coroutine.
     *
     * If the generator has thrown an exception it will be rethrown here.
     **/
    reference operator *() const noexcept {
        if (mGeneratorCoroutine.promise().exception()) {
            std::rethrow_exception(mGeneratorCoroutine.promise().exception());
        }
        return mGeneratorCoroutine.promise().value();
    }

    bool operator==(const GeneratorIterator &other) const noexcept {
        return mGeneratorCoroutine == other.mGeneratorCoroutine;
    }

    bool operator!=(const GeneratorIterator &other) const noexcept {
        return !(operator==(other));
    }

private:
    friend class QCoro::Generator<T>;

    /**
     * @brief Constructs an invalid iterator.
     **/
    explicit GeneratorIterator(std::nullptr_t) {}
    /**
     * @brief Constructs an iterator associated with the given generator coroutine.
     **/
    explicit GeneratorIterator(std::coroutine_handle<promise_type> generatorCoroutine)
        : mGeneratorCoroutine(generatorCoroutine)
    {}

    std::coroutine_handle<promise_type> mGeneratorCoroutine{nullptr};
};

} // namespace detail

/**
 * @brief A coroutine generator.
 *
 * The Generator class is returned from the generator coroutine. It's similar
 * to QCoro::Task<T> in the sense that it provides caller with an interface to
 * the generator coroutine.
 *
 * There is no standard interface for generators defined in the C++ standard.
 * Following the generators implemented in cppcoro (which is a big inspiration)
 * for QCoro), the Generator<T> provides begin() method to obtain an iterator
 * representing the current value produced by the generator. Each incrementation
 * of the iterator resumes the generator and provides next value. The end() method
 * can be used to obtain a past-end iterator to detect when the generator coroutine
 * has finished (no more values will be produced), unless the generator is infinite.
 *
 * When the Generator<T> object is destroyed, the associated generator coroutine is
 * also destroyed, even if it has not yet finished. All values allocated on stack of
 * the generator coroutine will be destroyed automatically.
 */
template<typename T>
class Generator {
public:
    using promise_type = detail::GeneratorPromise<T>;

    /**
     * @brief Destroys this Generator object and the associated generator coroutine.
     *
     * All values allocated on the generator stack are destroyed automatically.
     */
    ~Generator() {
        mGeneratorCoroutine.destroy();
    }

    /**
     * @brief Returns iterator "pointing" to the first value produced by the generator.
     *
     * If the generator coroutine did not produce any value and finished immediatelly,
     * the returned iterator will be equal to end().
     **/
    detail::GeneratorIterator<T> begin() {
        mGeneratorCoroutine.resume(); // generate first value
        if (mGeneratorCoroutine.promise().finished()) { // did not yield anything
            return detail::GeneratorIterator<T>{nullptr};
        }
        return detail::GeneratorIterator<T>{mGeneratorCoroutine};
    }

    /**
     * @brief Returns iterator indicating the past-last value produced by the generator.
     *
     * Can be used to check whether the generator have produced another value or
     * whether it has finished.
     **/
    detail::GeneratorIterator<T> end() {
        return detail::GeneratorIterator<T>{nullptr};
    }

private:
    friend QCoro::Generator<T> QCoro::detail::GeneratorPromise<T>::get_return_object();

    /**
     * @brief Constructs a new Generator object for given generator coroutine.
     *
     * This is called from GeneratorPromise<T>::get_return_object(), which is invoked
     * automatically by C++ when starting the generator coroutine.
     */
    explicit Generator(std::coroutine_handle<promise_type> generatorCoroutine)
        : mGeneratorCoroutine(generatorCoroutine)
    {}

    std::coroutine_handle<promise_type> mGeneratorCoroutine;
};

} // namespace QCoro

template<typename T>
QCoro::Generator<T> QCoro::detail::GeneratorPromise<T>::get_return_object() {
    using handle_type = std::coroutine_handle<typename QCoro::Generator<T>::promise_type>;
    return QCoro::Generator<T>(handle_type::from_promise(*this));
}
