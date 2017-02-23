#pragma once

#include <httplib/detail/common.hpp>

#include <boost/variant.hpp>

#include <type_traits>
#include <utility>


HTTPLIB_OPEN_NAMESPACE


namespace detail {

template<class T, class Tag>
struct tagged_value {
    T value;

    template<class... Args>
    explicit tagged_value(Args &&... args) :
        value(std::forward<Args>(args)...)
    { }

    tagged_value(const tagged_value &) = default;
    tagged_value(tagged_value &&) = default;

    tagged_value &operator=(const tagged_value &) = default;
    tagged_value &operator=(tagged_value &&) = default;
};


template<class T, bool CopyConstruct, bool CopyAssign>
struct copy_controller;

template<class T>
struct copy_controller<T, true, true> {
    T value;

    template<class... Args>
    explicit copy_controller(Args &&... args) :
        value(std::forward<Args>(args)...)
    { }

    copy_controller(const copy_controller &) = default;
    copy_controller(copy_controller &&) = default;

    copy_controller &operator=(const copy_controller &) = default;
    copy_controller &operator=(copy_controller &&) = default;
};

template<class T>
struct copy_controller<T, false, true> {
    T value;

    template<class... Args>
    explicit copy_controller(Args &&... args) :
        value(std::forward<Args>(args)...)
    { }

    copy_controller(const copy_controller &) = delete;
    copy_controller(copy_controller &&) = default;

    copy_controller &operator=(const copy_controller &) = default;
    copy_controller &operator=(copy_controller &&) = default;
};

template<class T>
struct copy_controller<T, true, false> {
    T value;

    template<class... Args>
    explicit copy_controller(Args &&... args) :
        value(std::forward<Args>(args)...)
    { }

    copy_controller(const copy_controller &) = default;
    copy_controller(copy_controller &&) = default;

    copy_controller &operator=(const copy_controller &) = delete;
    copy_controller &operator=(copy_controller &&) = default;
};

template<class T>
struct copy_controller<T, false, false> {
    T value;

    template<class... Args>
    explicit copy_controller(Args &&... args) :
        value(std::forward<Args>(args)...)
    { }

    copy_controller(const copy_controller &) = delete;
    copy_controller(copy_controller &&) = default;

    copy_controller &operator=(const copy_controller &) = delete;
    copy_controller &operator=(copy_controller &&) = default;
};


} // namespace detail


template<class Value, class Error>
class result {
public:
    using value_type = Value;
    using error_type = Error;

public:
    template<class Result, class... Args>
    friend Result make_result(Args &&... args);

    template<class Result, class... Args>
    friend Result make_error_result(Args &&... args);

public:
    result() :
        m_data(error_variant_t())
    { }

    result(const value_type &value) :
        m_data(value_variant_t(value))
    { }

    result(value_type &&value) :
        m_data(value_variant_t(std::move(value)))
    { }

    result(const result &) = default;
    result(result &&) = default;

    result &operator=(const result &) = default;
    result &operator=(result &&) = default;

    bool is_error() const {
        return boost::get<error_variant_t>(&m_data.value);
    }

    explicit operator bool() const {
        return !is_error();
    }

    const error_type &error() const {
        return boost::get<error_variant_t>(m_data.value).value;
    }

    error_type &error() {
        return boost::get<error_variant_t>(m_data.value).value;
    }

    const value_type &value() const {
        return boost::get<value_variant_t>(m_data.value).value;
    }

    value_type &value() {
        return boost::get<value_variant_t>(m_data.value).value;
    }

    const value_type &operator*() const {
        return value();
    }

    value_type &operator*() {
        return value();
    }

    const value_type *operator->() const {
        return &value();
    }

    value_type *operator->() {
        return &value();
    }

private:
    struct value_tag_t { };
    struct error_tag_t { };

private:
    template<class... Args>
    result(const value_tag_t &, Args &&... args) :
        m_data(value_variant_t(std::forward<Args>(args)...))
    { }

    template<class... Args>
    result(const error_tag_t &, Args &&... args) :
        m_data(error_variant_t(std::forward<Args>(args)...))
    { }

private:
    using value_variant_t = detail::tagged_value<value_type, value_tag_t>;

    using error_variant_t = detail::tagged_value<error_type, error_tag_t>;

    // This is to make better compiler error messages. Without this clang produces hundreds-lines-long
    // messages with boost::variant internals.
    using data_t = detail::copy_controller<
        boost::variant<error_variant_t, value_variant_t>,
        std::is_copy_constructible<value_type>::value && std::is_copy_constructible<error_type>::value,
        std::is_copy_assignable<value_type>::value && std::is_copy_assignable<error_type>::value
    >;

    data_t m_data;
};


template<class Result, class... Args>
Result make_result(Args &&... args) {
    return Result(typename Result::value_tag_t(), std::forward<Args>(args)...);
}


template<class Result, class... Args>
Result make_error_result(Args &&... args) {
    return Result(typename Result::error_tag_t(), std::forward<Args>(args)...);
}


template<class Value, class Error>
bool operator==(const result<Value, Error> &one, const result<Value, Error> &another) {
    if (one.is_error() && another.is_error()) {
        return one.error() == another.error();
    } else if (!one.is_error() && !another.is_error()) {
        return one.value() == another.value();
    } else {
        return false;
    }
}


template<class Value, class Error>
bool operator==(const result<Value, Error> &one, const Value &another) {
    return !one.is_error() && one.value() == another;
}


template<class Value, class Error>
bool operator==(const Value &one, const result<Value, Error> &another) {
    return another == one;
}


template<class Value, class Error>
bool operator!=(const result<Value, Error> &one, const result<Value, Error> &another) {
    return !(one == another);
}


template<class Value, class Error>
bool operator!=(const result<Value, Error> &one, const Value &another) {
    return one.is_error() || one.value() != another;
}


template<class Value, class Error>
bool operator!=(const Value &one, const result<Value, Error> &another) {
    return another != one;
}


HTTPLIB_CLOSE_NAMESPACE
