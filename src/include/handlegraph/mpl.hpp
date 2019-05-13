#ifndef HANDLEGRAPH_MPL_HPP_INCLUDED
#define HANDLEGRAPH_MPL_HPP_INCLUDED

/** \file
 * Defines metaprogramming tools we need so that we can have types that
 * represent a combination of interface "traits" from a closed set.
 *
 * Accomplishes this by using templates on bit sets that are the flags for
 * which traits are implemented.
 */

#include <cstdint>
#include <utility>
#include <functional>
#include <limits>

namespace handlegraph {

// Define the type we will use for our bit flag sets
// Must have one bit per trait at least.
using bits_t = uint8_t;

// Define some templates to map feature traits to and from but numbers.
// These implementations just exist to be specialized.
template<typename Feature>
struct feature_number {
    // We have a zero value here, even though we represent an invalid feature.
    // It will get caught by an informative static assert later.
    static const bits_t value = 0;
};
template<bits_t Bit>
struct feature {
};


/// Define a macro to forward-declare a feature trait and map it to and from a
/// bit using templates.
///
/// Each trait can imply other traits; we make sure to inherit the right classes
/// to match when just those traits are requested.
#define HANDLEGRAPH_TRAIT(Feature, Bit, Implied) \
struct Feature; \
template<> \
struct feature_number<Feature>{ \
    static const bits_t value = (Bit) | (Implied); \
}; \
template<> \
struct feature<Bit> { \
    using type = Feature; \
};


// Invoke it for all the feature traits we have, and all the traits we know they pull in.
HANDLEGRAPH_TRAIT(HandleGraph, 1, 0);
HANDLEGRAPH_TRAIT(Mutable, 2, 0);
HANDLEGRAPH_TRAIT(Deletable, 4, 2);
HANDLEGRAPH_TRAIT(PathSupport, 8, 0);
HANDLEGRAPH_TRAIT(MutablePaths, 16, 8);
HANDLEGRAPH_TRAIT(DeletablePaths, 32, 16 | 8);

// Now we use a bits_t to hold a set of interfaces as a bitmap.
// We then inherit everything from the set in bit order.

// We have something to OR together all the flags for a list of interfaces.
template<typename... Empty>
struct bitmap_of {
    static constexpr bits_t value = 0;
};

template<typename First, typename... Rest>
struct bitmap_of<First, Rest...> {
    // Make sure only defined values go in
    static_assert(feature_number<First>::value != 0, "Non-trait type passed to HandleGraphWith<>");
    static constexpr bits_t value = feature_number<First>::value | bitmap_of<Rest...>::value;
};

/// Get the highest set bit, at compile time.
/// TODO: If we run out of bits, make this use a wider type.
template<bits_t x>
inline constexpr bits_t highest_set_bit()
{
    // Loop down from the high non-sign bit.
    // If we want *all* the bits we need to use a unsigned type.
    bits_t i = 1 << (std::numeric_limits<bits_t>::digits - 1);
    for (; i != 0; i = i >> 1) {
        if ((x & i) != 0) {
            break;
        }
    }
    return i;
}

// To debug this, pass the result to an incomplete template. See <https://stackoverflow.com/a/2008577>

// Make sure it works
static_assert(highest_set_bit<0>() == 0);
static_assert(highest_set_bit<1>() == 1);
static_assert(highest_set_bit<2>() == 2);
static_assert(highest_set_bit<4>() == 4);
static_assert(highest_set_bit<10>() == 8);

static_assert(std::is_same<feature<1>::type, HandleGraph>::value);

/// Given a bitmap of traits to inherit from, inherit from all of them in a consistent order.
template<bits_t bitmap>
struct InheritsFromBits :
    public virtual feature<highest_set_bit<bitmap>()>::type,
    public virtual InheritsFromBits<bitmap ^ highest_set_bit<bitmap>()> {
};

template<>
struct InheritsFromBits<0> {
};

/// Now we have a class that inherits from itself for all subsets of set bits,
/// and also from InheritsFromBits for its set bits.
///
/// To accomplish this we want to do mutual recursion, but we can't do that,
/// because a class is always incomplete until the end of its definition, and
/// you can't inherit from an incomplete class. So we fake it with another
/// template parameter which tracks if we're the normal version, or the
/// mutual-recusrion version that exists only to hit the normal version with
/// the right arguments.
///
/// It can't inherit from any intermediate things, because it's not enough for
/// the user's requested type and the implementing type to share a common base
/// class. The requested type must *be* a base class of the implementing type.
///
/// We also have a stop_now so we can stop, instead of trying to recurse to
/// unset non-set bits and not making progress.
/// TODO: this is ugly. Clean it up.
template<bits_t bitmap, bits_t to_clear = 0, bool is_normal = true, bool stop_now = false>
struct InheritsFromBitSubsets :
    // Inherit from the actual class that gets us this interface combo
    public virtual InheritsFromBits<bitmap>,
    // Drop into the mutually-recursive version that will come back and depend on us with all the bitmap subsets *except* the full one.
    public virtual InheritsFromBitSubsets<bitmap, highest_set_bit<bitmap>(), false> {
};

/// Mutually recusring version.
/// Inherit from InheritsFromBitSubsets with to_clear cleared, and then knock
/// to_clear down by 1 place (independent of set bits) and recurse ourselves.
template<bits_t base_bitmap, bits_t to_clear>
struct InheritsFromBitSubsets<base_bitmap, to_clear, false, false> :
    // Recurse back to the normal version with the bit cleared.
    // Unless the bit isn't set in which case we wouldn't make progress by clearing it.
    // In which case we jump to the base case.
    public virtual InheritsFromBitSubsets<base_bitmap & ~to_clear, 0, true, (base_bitmap & ~to_clear) == base_bitmap>,
    // Recurse on ourselves to clear the next bit.
    // We ignore whether it is set or not and try to clear it always.
    public virtual InheritsFromBitSubsets<base_bitmap, to_clear/2, false> {
};

/// Mutualy recursive base case: no more bits left to clear.
/// Don't need to recurse back because we already hit the full bitmap on entry.
template<bits_t base_bitmap>
struct InheritsFromBitSubsets<base_bitmap, 0, false, false> {
};

/// Early stopping base case.
template<bits_t base_bitmap, bits_t to_clear>
struct InheritsFromBitSubsets<base_bitmap, to_clear, true, true> {
};


/// Class that inherits from all subsets of the given list of traits.
/// Traits must have bits assigned to them via HANDLEGRAPH_TRAIT.
/// Must be a using so we can emiminate trait order.
template<typename... Traits>
using InheritsAll = InheritsFromBitSubsets<bitmap_of<Traits...>::value>;


}

#endif
