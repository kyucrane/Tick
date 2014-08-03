/*=============================================================================
    Copyright (c) 2014 Paul Fultz II
    builder.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef TICK_GUARD_BUILDER_H
#define TICK_GUARD_BUILDER_H

#include <tick/detail/matches.h>
#include <tick/detail/pp.h>
#include <tick/integral_constant.h>

namespace tick {

namespace detail {
template<class T1 = void, class T2 = void, class T3 = void, class T4 = void, class T5 = void>
struct holder
{
    typedef void type;
};

template<template<class...> class... Templates>
struct template_holder
{
    typedef void type;
};

struct void_ {};
template<class T> T&& operator,(T&& x, void_);

template<class T>
struct bare
: std::remove_cv<typename std::remove_reference<T>::type>
{};

template<class T>
struct avoid
: std::conditional<
    std::is_same<typename bare<T>::type, void_>::value,
    void,
    T
>
{};

template<class T>
struct is_void
: std::is_same<T, void>
{};

template<class T>
struct is_void<std::is_same<T, void>>
: std::true_type
{};

template<class T>
struct is_void<std::is_same<void, T>>
: std::true_type
{};

template<typename T>
int valid_expr(T &&);

struct base_requires
{
    template<class... Ts>
    int requires_(Ts&&...);
};

template<class T>
struct always_false
: tick::integral_constant<bool, false>
{};

template<class T, class U>
struct return_matches
: detail::matches<typename detail::avoid<U>::type,T>
{
    static_assert(!detail::is_void<T>::value, "Void can't be used for returns");
};

}

// Deprecated: TICK_VALID is no loneger necessary
#define TICK_VALID(...) decltype(__VA_ARGS__)


struct ops : tick::local_placeholders
{

    template<typename T, typename U>
    static auto returns(U &&) ->
        typename std::enable_if<detail::return_matches<T,U>::value, int>::type;

// A macro to provide better compatibility for gcc 4.6
#define TICK_RETURNS(expr, ...) returns<__VA_ARGS__>((expr, tick::detail::void_()))
// #define TICK_RETURNS(expr, ...) returns<__VA_ARGS__>(expr)

    template<
        class T, 
        class U = void, 
        class Enable = typename std::enable_if<detail::matches<T,U>::value>::type>
    class has_type {};

    template<template<class...> class Template>
    class has_template {};

    // TODO: Try to make a compile error if T::value doesn't exists
    template<
        class T, 
        class Enable = typename std::enable_if<T::value>::type>
    class is_true {};

    template<
        class T, 
        class Enable = typename std::enable_if<not T::value>::type>
    class is_false {};

};

template<class... Traits>
struct base_traits;

template<class Trait, class... Traits>
struct base_traits<Trait, Traits...>
: tick::integral_constant<bool, Trait::value and base_traits<Traits...>::value>
{
    typedef base_traits<Trait, Traits...> base_traits_type;
};
template<>
struct base_traits<>
: std::true_type
{
    typedef base_traits<> base_traits_type;
};
template<class... Lambdas>
struct refines
{
    template<class... Ts>
    struct tick_trait_base_apply
    : base_traits<typename tick::detail::replace_args<Lambdas, Ts...>::type...>
    {
    };
};

template<class T, class Enable=void>
struct refine_traits
{
    template<class... Ts>
    struct apply
    : base_traits<>
    {
    };
};

template<class T>
struct refine_traits<T, typename detail::template_holder<T::template tick_trait_base_apply>::type>
{
    template<class... Ts>
    struct apply
    : T::template tick_trait_base_apply<Ts...>
    {

    };
};

template<class T, class Enable = void>
struct base_traits_type
{
    typedef base_traits<> type;
};

template<class T>
struct base_traits_type<T, typename detail::holder<
    typename T::base_traits_type
>::type>
{
    typedef typename T::base_traits_type type;
};

template<class Trait, class X = void>
struct models 
: std::false_type
{};
// TODO: Add axioms
template<class Trait, class... Ts>
struct models<Trait(Ts...), typename detail::holder<
    decltype(std::declval<Trait>().requires_(std::declval<Ts>()...))
>::type>
: refine_traits<Trait>::template apply<Ts...>
{};

// Deprecated: Provided here for backwards compatiblity
template<class Trait>
struct trait : models<Trait>
{};

#define TICK_TRAIT_REFINES(name, ...) \
struct tick_private_trait_base_ ## name : tick::ops \
{ typedef tick::refines<__VA_ARGS__> type; }; \
struct tick_private_trait_ ## name; \
template<class... T> \
struct name \
: tick::models<tick_private_trait_ ## name(T...)> \
{}; \
struct tick_private_trait_ ## name \
: tick::detail::base_requires, tick::ops, tick_private_trait_base_ ## name::type

#define TICK_TRAIT(...) \
    TICK_PP_EXPAND( \
        TICK_TRAIT_REFINES TICK_IIF(TICK_PP_ARGS_IS_SINGLE(__VA_ARGS__)) \
        ( \
            (__VA_ARGS__,), \
            (__VA_ARGS__)) \
        )

// Add support for zlang
#define ZLANG_tick_trait (TICK_TRAIT)

}

#endif