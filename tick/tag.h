/*=============================================================================
    Copyright (c) 2014 Paul Fultz II
    tag.h
    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#ifndef TICK_GUARD_TAG_H
#define TICK_GUARD_TAG_H

#include <tick/builder.h>

namespace tick {

template<template<class...> class Trait>
struct tag;

template<class T>
struct most_refined;

namespace detail {

template<class T, class=void>
struct get_refinements
{ typedef refines<> type; };

template<class...Ts, template<class...> class Template>
struct get_refinements<Template<Ts...>, typename holder<
    typename Template<no_check>::type::tick_trait_refinements
>::type>
{
    typedef typename Template<no_check>::type::tick_trait_refinements type;
};

template<class T>
struct make_tag;

template<class... Ts, template<class...> class Template>
struct make_tag<Template<Ts...>>
: tag<Template>
{};

template<class T>
struct make_tags;

template<class... Ts>
struct make_tags<refines<Ts...>>
: make_tag<Ts>...
{};

template<class Refinements, class... Ts>
struct get_tags;

template<class... Lambdas, class... Ts>
struct get_tags<refines<Lambdas...>, Ts...>
: most_refined<typename replace_args<Lambdas, Ts...>::type>...
{};

template<class T>
struct get_base_tags;

template<class... Ts, template<class...> class Trait>
struct get_base_tags<Trait<Ts...>>
: get_tags<typename get_refinements<Trait<Ts...>>::type, Ts...>
{};

}

template<template<class...> class Trait>
struct tag
: detail::make_tags<typename detail::get_refinements<Trait<detail::no_check>>::type>
{};

template<class T>
struct most_refined
: std::conditional<T::value, detail::make_tag<T>, detail::get_base_tags<T>>::type
{};

}

#endif