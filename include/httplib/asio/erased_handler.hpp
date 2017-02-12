#pragma once

#include <httplib/detail/common.hpp>

#include <boost/asio/handler_alloc_hook.hpp>
#include <boost/asio/handler_continuation_hook.hpp>
#include <boost/asio/handler_invoke_hook.hpp>

#include <cstdlib>
#include <utility>


HTTPLIB_OPEN_NAMESPACE

namespace detail {

template<class R, class... Args>
class erased_handler_base {
public:
    virtual ~erased_handler_base() { }

    virtual R operator()(Args... args) = 0;
    virtual void *allocate(std::size_t) = 0;
    virtual void deallocate(void *, std::size_t) = 0;
    virtual bool is_continuation() = 0;
    virtual void invoke(std::function<void()>) = 0;

    virtual std::unique_ptr<erased_handler_base> clone() = 0;
};

template<class F, class R, class... Args>
class erased_handler_impl: public erased_handler_base<R, Args...> {
public:
    erased_handler_impl(F f) :
        m_function(f)
    { }

    R operator()(Args... args) override {
        return m_function(std::move(args)...);
    }

    void *allocate(std::size_t size) override {
        using boost::asio::asio_handler_allocate;
        return asio_handler_allocate(size, &m_function);
    }

    void deallocate(void *ptr, std::size_t size) override {
        using boost::asio::asio_handler_deallocate;
        asio_handler_deallocate(ptr, size, &m_function);
    }

    bool is_continuation() override {
        using boost::asio::asio_handler_is_continuation;
        return asio_handler_is_continuation(&m_function);
    }

    void invoke(std::function<void()> callable) override {
        using boost::asio::asio_handler_invoke;
        asio_handler_invoke(callable, &m_function);
    }

    std::unique_ptr<erased_handler_base<R, Args...>> clone() override {
        return std::make_unique<erased_handler_impl>(m_function);
    }

private:
    F m_function;
};

} // namespace detail


template<class Signature>
class erased_handler;

template<class R, class... Args>
class erased_handler<R(Args...)> {
public:
    template<class F>
    erased_handler(F f) :
        m_impl(std::make_unique<detail::erased_handler_impl<F, R, Args...>>(std::move(f)))
    { }

    erased_handler(const erased_handler &other) :
        m_impl(other.m_impl->clone())
    { }

    erased_handler(erased_handler &&other) :
        m_impl(std::move(other.m_impl))
    { }

    erased_handler *operator=(const erased_handler &other) {
        m_impl = other.m_impl->clone();
        return *this;
    }

    erased_handler *operator=(erased_handler &&other) {
        m_impl = std::move(other.m_impl);
        return *this;
    }

    template<class... Args2>
    R operator()(Args2 &&... args) {
        return (*m_impl)(std::forward<Args2>(args)...);
    }


    friend void *asio_handler_allocate(std::size_t size, erased_handler *context) {
        return context->m_impl->allocate(size);
    }

    friend void asio_handler_deallocate(void *pointer, std::size_t size, erased_handler *context) {
        context->m_impl->deallocate(pointer, size);
    }

    friend bool asio_handler_is_continuation(erased_handler *context) {
        return context->m_impl->is_continuation();
    }

    template<class Callable>
    friend void asio_handler_invoke(Callable &function, erased_handler *context) {
        context->m_impl->invoke(function);
    }

    template<class Callable>
    friend void asio_handler_invoke(const Callable &function, erased_handler *context) {
        context->m_impl->invoke(function);
    }

private:
    std::unique_ptr<detail::erased_handler_base<R, Args...>> m_impl;
};

HTTPLIB_CLOSE_NAMESPACE
