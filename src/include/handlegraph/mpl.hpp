#ifndef HANDLEGRAPH_MPL_HPP_INCLUDED
#define HANDLEGRAPH_MPL_HPP_INCLUDED

/** \file
 * Defines metaprogramming tools we need so that we can have types that represent a combination of interfaces.
 */

#include <cstdint>
#include <utility>
#include <functional>

namespace handlegraph {

// Define feature flags/things to inherit
struct Base {};
struct Mutable {};
struct Paths {};
struct MutablePaths {};
struct Deletable {};

// Map from features to numbers with a trait
// Also map back

template<typename T> 
struct feature_number { 
    static const int value = 0;
};

template<int N> 
struct feature {
    using type = Base;
};

template<> 
struct feature_number<Mutable>{ 
    static const int value = 1; 
};

template<> 
struct feature<1> {
    using type = Mutable;
};

template<> 
struct feature_number<Paths>{ 
    static const int value = 2; 
};

template<> 
struct feature<2> {
    using type = Paths;
};

template<> 
struct feature_number<MutablePaths>{ 
    static const int value = 3; 
};

template<> 
struct feature<3> {
    using type = MutablePaths;
};

template<> 
struct feature_number<Deletable>{ 
  static const bool value = 4; 
};

template<> 
struct feature<4> {
    using type = Deletable;
};

// Implement compile time bubble sort like https://stackoverflow.com/a/19282772 but with templates
// We do ascending sort because it is easier. We can look at empty tails as 0.


// We need a way to get the max.

// This only hits the empty pack case
template<typename... Empty>
struct max_value {
    static constexpr int value = 0;
};

// This hits any pack that is nonempty
template<typename First, typename... Rest>
struct max_value<First, Rest...> {
    // Do a compile time conditional comparison
    static constexpr int value = feature_number<First>::value > max_value<Rest...>::value ?
        feature_number<First>::value :
        max_value<Rest...>::value;
};

// We need a way to get the max value that is below another value
template<int limit, typename... Empty>
struct max_value_below {
    static constexpr int value = 0;
};

template<int limit, typename First, typename... Rest>
struct max_value_below<limit, First, Rest...> {
    // Get the max below from rest, and 0 if we're above it or our value if we're below it
    static constexpr int from_rest = max_value_below<limit, Rest...>::value;
    static constexpr int from_us = feature_number<First>::value < limit ? feature_number<First>::value : 0;
    
    // And return the greater one.
    static constexpr int value = from_us > from_rest ? from_us : from_rest;
};

// We need empty list detection
template<typename... Empty>
struct is_empty {
    static constexpr bool value = true;
};

template<typename First, typename... Rest>
struct is_empty<First, Rest...> {
    static constexpr bool value = false;
};

// And we need a way to get the value for the first element
template<typename... Empty>
struct get_first_value {
    static constexpr int value = 0;
};

template<typename First, typename... Rest>
struct get_first_value<First, Rest...> {
    static constexpr int value = feature_number<First>::value;
};

// And we need a way to get the type for the first element
template<typename... Empty>
struct get_first_type {
};

template<typename First, typename... Rest>
struct get_first_type<First, Rest...> {
    using type = First;
};

// Now to inherit everything, we just inherit the max, and then the max below that...

template<int n>
struct InheritOne : public feature<n>::type {};

template<int limit, typename... Empty>
struct InheritAllBelow {
    using type = Base;
};

// Zero case does nothing
template< typename... Empty>
struct InheritAllBelow<0, Empty...> {
    using type = Base;
};

template<typename A, typename B>
struct InheritBoth : public A, public B {
};

template<int limit, typename First, typename... Rest>
struct InheritAllBelow<limit, First, Rest...> {

    using type = InheritBoth<InheritOne<max_value_below<limit, First, Rest...>::value>, InheritAllBelow<max_value_below<limit, First, Rest...>::value, First, Rest...>>;

};

// Zero case does nothing
template<typename First, typename... Rest>
struct InheritAllBelow<0, First, Rest...> {
    using type = Base;
};

template<typename... Empty>
struct InheritAll {
    using type = Base;
};

// We provide a type to inherit from that inherits from all the given types in sorted order.
template<typename First, typename... Rest>
struct InheritAll<First, Rest...>   {
    using type = InheritAllBelow<9999, First, Rest...>;
};

// Now we use it

struct PathAndMutable : public InheritAll<Mutable, Paths>::type {
};

}

#endif
