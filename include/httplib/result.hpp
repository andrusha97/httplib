#pragma once

#include <httplib/detail/common.hpp>

#include <boost/variant.hpp>

#include <utility>


HTTPLIB_OPEN_NAMESPACE


namespace detail {

template<class T, class Tag>
struct tagged_value {
    T value;

    template<class... Args>
    tagged_value(Args &&... args) :
        value(std::forward<Args>(args)...)
    { }
};

} // namespace detail


template<class Value, class Error>
class result {
public:
    using value_type = Value;
    using error_type = Error;

public:
    template<class V, class E, class... Args>
    friend result<V, E> make_result(Args &&... args);

    template<class V, class E, class... Args>
    friend result<V, E> make_error_result(Args &&... args);

public:
    result() :
        m_data(error_variant_t())
    { }

    result(value_type value) :
        m_data(value_variant_t(std::move(value)))
    { }

    result(error_type error) :
        m_data(error_variant_t(std::move(error)))
    { }

    bool is_error() const {
        return boost::get<error_variant_t>(&m_data);
    }

    explicit operator bool() const {
        return !is_error();
    }

    const error_type &error() const {
        return boost::get<error_variant_t>(m_data).value;
    }

    error_type &error() {
        return boost::get<error_variant_t>(m_data).value;
    }

    const value_type &value() const {
        return boost::get<value_variant_t>(m_data).value;
    }

    value_type &value() {
        return boost::get<value_variant_t>(m_data).value;
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

    boost::variant<error_variant_t, value_variant_t> m_data;
};


template<class V, class E, class... Args>
result<V, E> make_result(Args &&... args) {
    return result<V, E>(typename result<V, E>::value_tag_t(), std::forward<Args>(args)...);
}


template<class V, class E, class... Args>
result<V, E> make_error_result(Args &&... args) {
    return result<V, E>(typename result<V, E>::error_tag_t(), std::forward<Args>(args)...);
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
bool operator!=(const result<Value, Error> &one, const result<Value, Error> &another) {
    return !(one == another);
}


HTTPLIB_CLOSE_NAMESPACE
