#ifndef HANDLEGRAPH_ERROR_HANDLING_HPP_INCLUDED
#define HANDLEGRAPH_ERROR_HANDLING_HPP_INCLUDED

/** \file
 * Defines HANDLEGRAPH_THROW, the macro libhandlegraph uses to report
 * unrecoverable errors, so that the error-reporting mechanism can be selected
 * at build time.
 *
 * By default, HANDLEGRAPH_THROW(ExceptionType, message) throws
 * ExceptionType(message), matching libhandlegraph's traditional behavior.
 * Some environments (notably Bazel builds embedding libhandlegraph in
 * exceptions-free code, such as Google's DeepVariant) need to build with
 * -fno-exceptions. Defining HANDLEGRAPH_NO_EXCEPTIONS at build time switches
 * HANDLEGRAPH_THROW to a non-throwing fatal-error path instead: it logs the
 * message with Abseil's ABSL_LOG(FATAL) if HANDLEGRAPH_USE_ABSEIL_LOGGING is
 * also defined, or otherwise prints it to stderr and calls std::abort().
 * Either way, ExceptionType is not evaluated, so it does not need to be a
 * complete type when exceptions are disabled.
 */

#if defined(HANDLEGRAPH_NO_EXCEPTIONS)

#if defined(HANDLEGRAPH_USE_ABSEIL_LOGGING)
#include "absl/log/absl_log.h"
#define HANDLEGRAPH_THROW(exception_type, message) ABSL_LOG(FATAL) << (message)
#else
#include <cstdlib>
#include <iostream>
#define HANDLEGRAPH_THROW(exception_type, message) \
    do { std::cerr << (message) << std::endl; std::abort(); } while (0)
#endif

#else

#include <stdexcept>
#define HANDLEGRAPH_THROW(exception_type, message) throw exception_type(message)

#endif

#endif
