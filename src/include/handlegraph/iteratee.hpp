#ifndef HANDLEGRAPH_ITERATEE_HPP_INCLUDED
#define HANDLEGRAPH_ITERATEE_HPP_INCLUDED

/** \file 
 * Defines tools for implementing the iteratee pattern, with optional eraly stopping.
 *
 * The basic idea of the iteratee pattern is that we have a function that takes
 * an "iteratee" lambda and calls it, possibly in parallel, with a bunch of
 * items. The iteratee can either be void-returning or bool-returning. If it is
 * bool-returning, when it returns false, the iteration will stop.
 *
 * This is tricky to implement in C++ because we can't overload on bool-vs-void
 * return type of std::function, as described in
 * <https://stackoverflow.com/a/6194623>; we use templates that get
 * instantiated over unique passed-in lambda types to work around this.
 *
 * HandleGraph interfaces will require a for_each_whatever_impl virtual
 * protected method to be implemented by implementations, always operating oin
 * a bool-returning iteratee, and expose public non-virtual for_each_whatever
 * templates that manage the wraping of void-returning iteratees.
 */
 
#include <functional>

namespace handlegraph {

/// This template has a static method that takes a callable on items and
/// returns a wrapped version that returns the calable's returned bool, or true
/// for void-returning callables.
template<typename Iteratee, typename Iterated,
    typename IterateeReturnType = decltype(std::declval<Iteratee>().operator()(std::declval<handle_t>()))>
struct BoolReturningWrapper {
    static inline std::function<bool(const Iterated&)> wrap(const Iteratee& iteratee);
};


////////////////////////////////////////////////////////////////////////////
// Template Implementations
////////////////////////////////////////////////////////////////////////////

/// This specialization handles wrapping void-returning callables.
template<typename Iteratee, typename Iterated>
struct BoolReturningWrapper<Iteratee, Iterated, void> {
    static inline std::function<bool(const Iterated&)> wrap(const Iteratee& iteratee) {
        return [&](const Iterated& item){
            iteratee(item);
            return true;
        };
    }
};

/// This specialization handles wrapping bool-returning callables.
template<typename Iteratee, typename Iterated>
struct BoolReturningWrapper<Iteratee, Iterated, bool> {
    static inline std::function<bool(const Iterated&)> wrap(const Iteratee& iteratee) {
        return [&](const Iterated& item){
            return iteratee(item);
        };
    }
};


}

#endif

