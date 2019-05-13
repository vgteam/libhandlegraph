#ifndef HANDLEGRAPH_MPL_HPP_INCLUDED
#define HANDLEGRAPH_MPL_HPP_INCLUDED

/** \file
 * Defines metaprogramming tools we need so that we can have types that represent a combination of interfaces.
 */

#include <cstdint>
#include <utility>
#include <functional>

namespace handlegraph {

// Define some templates to map feature traits to and from but numbers.
// These implementations just exist to be specialized.
template<typename Feature>
struct feature_number {
};
template<int Bit>
struct feature {
};


// Define a macro to forward-declare a feature trait and map it to and from a bit using templates.
#define HANDLEGRAPH_TRAIT(Feature, Bit) \
struct Feature; \
template<> \
struct feature_number<Feature>{ \
    static const int value = Bit; \
}; \
template<> \
struct feature<Bit> { \
    using type = Feature; \
};


// Invoke it for all the feature traits we have
HANDLEGRAPH_TRAIT(Mutable, 1);
HANDLEGRAPH_TRAIT(Deletable, 2);
HANDLEGRAPH_TRAIT(Path, 4);
HANDLEGRAPH_TRAIT(MutablePath, 8);
HANDLEGRAPH_TRAIT(DeletablePath, 16);

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

/// Get the highest set bit, at compile time
template<int x>
inline constexpr int highest_set_bit()
{
    for (int i = 1<<10; i > 0; i = i/2) {
        if (x & i) {
            return i;
        }
    }
    return 0;
}

/// Given a bitmap of traits to inherit from, inherit from all of them in a consistent order.
template<int bitmap>
struct InheritsFromBits : public feature<highest_set_bit<bitmap>()>, public InheritsFromBits<bitmap ^ highest_set_bit<bitmap>()> {
};

template<>
struct InheritsFromBits<0> {
};

/// Compute a type that inherits from all of the specified traits.
template<typename First, typename... Rest>
struct inherit_all {
    using type = InheritsFromBits<bitmap_of<First, Rest...>::value>;
};

// Now we use it

struct Mutable {};
struct Path {};
struct MutablePath {};

struct PathAndMutable : public inherit_all<Mutable, Path>::type {
};

struct MutablePathAndMutable : public inherit_all<Mutable, Path, MutablePath>::type {
};

}

#endif
