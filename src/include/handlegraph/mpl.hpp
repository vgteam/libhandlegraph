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
HANDLEGRAPH_TRAIT(BaseHandleGraph, 1);
HANDLEGRAPH_TRAIT(Mutable, 2);
HANDLEGRAPH_TRAIT(Deletable, 4);
HANDLEGRAPH_TRAIT(Path, 8);
HANDLEGRAPH_TRAIT(MutablePath, 16);
HANDLEGRAPH_TRAIT(DeletablePath, 32);

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

/// Get the highest set bit, at compile time.
/// TODO: If we run out of bits, make this use a wider type.
template<int x>
inline constexpr int highest_set_bit()
{
    for (int i = 1 << 31; i > 0; i = i/2) {
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

template<int base_bitmap, int to_clear>
struct InheritsFromBitSubsetsClearingEach;

/// Now we have a class that inherits from itself for all subsets of set bits,
/// and also from InheritsFromBits for its set bits.
///
/// To accomplish this we do some mutual recursion.
///
/// It can't inherit from any intermediate things, because it's not enough for
/// the user's requested type and the implementing type to share a common base
/// class. The requested type must *be* a base class of the implementing type.
template<int bitmap>
struct InheritsFromBitSubsets : public virtual InheritsFromBits<bitmap>, public virtual InheritsFromBitSubsetsClearingEach<bitmap, highest_set_bit<bitmap>()> {
};

/// Inherit from InheritsFromBitSubsets with to_clear cleared, and then knock
/// to_clear down by 1 place (independent of set bits) and recurse ourselves.
template<int base_bitmap, int to_clear>
struct InheritsFromBitSubsetsClearingEach :
    public virtual InheritsFromBitSubsets<~(~base_bitmap & ~to_clear)>,
    public virtual InheritsFromBitSubsetsClearingEach<base_bitmap, to_clear/2> {
};

/// Base case: no more bits left to clear.
/// Don't need to recurse back because we already hit the full bitmap on entry.
template<int base_bitmap>
struct InheritsFromBitSubsetsClearingEach<base_bitmap, 0> {
};


/// Class that inherits from all subsets of the given list of traits.
/// Traits must have bits assigned to them via HANDLEGRAPH_TRAIT.
/// Must be a using so we can emiminate trait order.
template<typename... Traits>
using InheritsAll = InheritsFromBitSubsets<bitmap_of<Traits...>::value>;

// Now we use it

struct Mutable {};
struct Path {};
struct MutablePath {};

using PathAndMutable = InheritsAll<Mutable, Path>;

using MutablePathAndMutable = InheritsAll<Mutable, Path, MutablePath>;

// Make sure it worked

// We need to implement each trait
static_assert(std::is_base_of<InheritsAll<Mutable>, PathAndMutable>::value);
static_assert(std::is_base_of<InheritsAll<Path>, PathAndMutable>::value);
static_assert(std::is_base_of<InheritsAll<Path>, PathAndMutable>::value);

// We need to implement all orders of traits
static_assert(std::is_base_of<InheritsAll<Mutable, Path>, PathAndMutable>::value);
static_assert(std::is_base_of<InheritsAll<Path, Mutable>, PathAndMutable>::value);

// We need to implement subsets of traits
static_assert(std::is_base_of<PathAndMutable, MutablePathAndMutable>::value);

}

#endif
