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

/**
 * Tool to get the return type of a function.
 * See for example <https://stackoverflow.com/a/7943765>
 * For anything that's possibly a reference to something with with operator(), delegate to the return type of that operator.
 */
template<typename Function>
struct return_type_of : public return_type_of<decltype(&std::remove_reference<Function>::type::operator())>
{};

/**
 * Tool to get the return type of a const member function.
 */
template<typename Owner, typename Returns, typename... Args>
struct return_type_of<Returns(Owner::*)(Args...) const> {
    using type = Returns;
};

/**
 * Tool to get the return type of a non-const member function.
 */
template<typename Owner, typename Returns, typename... Args>
struct return_type_of<Returns(Owner::*)(Args...)> {
    using type = Returns;
};

/**
 * Tool to get the return type of a free function.
 */
template<typename Returns, typename... Args>
struct return_type_of<Returns(*)(Args...)> {
    using type = Returns;
};

// Then try and pick a different wrapper implementation depending on if the function being wrapped returns bool or not.

/**
 * Specializations of this class have a wrap() static method that takes a
 * function and returns a version of that function that returns a bool.
 */
template<typename Iteratee, bool ReturnsBool = std::is_same<typename return_type_of<Iteratee>::type, bool>::value>
struct BoolReturningWrapper {};

/**
 * Wrap a function that returns bool to return bool: a no-op.
 */
template<typename Iteratee>
struct BoolReturningWrapper<Iteratee, true> {
    static auto wrap(const Iteratee& iteratee) {
        return iteratee;
    }
};

/**
 * Wrap a function that returns anything other than bool to return bool.
 */
template<typename Iteratee>
struct BoolReturningWrapper<Iteratee, false> {
    static auto wrap(const Iteratee& iteratee) {
        return [&](auto&&... args) -> bool {
            iteratee(std::forward<decltype(args)>(args)...);
            return true;
        };
    }
};




}

#endif

