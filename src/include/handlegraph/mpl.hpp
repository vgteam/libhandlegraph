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
    static const int value = 4; 
};

template<> 
struct feature<4> {
    using type = MutablePaths;
};

template<> 
struct feature_number<Deletable>{ 
  static const bool value = 8; 
};

template<> 
struct feature<8> {
    using type = Deletable;
};


// Now we use an int to hold a set of interfaces as a bitmap.
// We then inherit everything from the set in bit order.

// We have something to OR together all the flags for a list of interfaces.
template<typename... Empty>
struct bitmap_of {
    static constexpr int value = 0;
};

template<typename First, typename... Rest>
struct bitmap_of<First, Rest...> {
    static constexpr int value = feature_number<First>::value | bitmap_of<Rest...>::value;
};

// We have something to get the highest set bit

template<int x>
constexpr int highest_set_bit()
{
    for (int i = 1<<10; i > 0; i = i/2) {
        if (x & i) {
            return i;
        }
    }
    return 0;
}

template<int bitmap>
struct InheritsFromBits : public feature<highest_set_bit<bitmap>()>, public InheritsFromBits<bitmap ^ highest_set_bit<bitmap>()> {
};

template<>
struct InheritsFromBits<0> {
};





// We provide a type to inherit from that inherits from all the given types in sorted order.
template<typename First, typename... Rest>
struct inherit_all {
    using type = InheritsFromBits<bitmap_of<First, Rest...>::value>;
};

// Now we use it

struct PathAndMutable : public inherit_all<Mutable, Paths>::type {
};

}

#endif
