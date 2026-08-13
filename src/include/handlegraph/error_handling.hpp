#ifndef HANDLEGRAPH_ERROR_HANDLING_HPP_INCLUDED
#define HANDLEGRAPH_ERROR_HANDLING_HPP_INCLUDED

/** \file
 * Defines HANDLEGRAPH_THROW, the macro libhandlegraph uses to report
 * unrecoverable errors, so that the error-reporting mechanism can be selected
 * at build time.
 *
 * HANDLEGRAPH_THROW(exception) throws the given exception object, unless
 * HANDLEGRAPH_NO_EXCEPTIONS is defined, in which case it logs exception.what()
 * and calls std::abort() instead (via Abseil if HANDLEGRAPH_USE_ABSEIL_LOGGING
 * is set).
 */

// HANDLEGRAPH_THROW always constructs its argument, even when not thrown.
#include <stdexcept>

#if defined(HANDLEGRAPH_NO_EXCEPTIONS)

#if defined(HANDLEGRAPH_USE_ABSEIL_LOGGING)
#include "absl/log/absl_log.h"
#define HANDLEGRAPH_THROW(exception) ABSL_LOG(FATAL) << (exception).what()
#else
#include <cstdlib>
#include <iostream>
#define HANDLEGRAPH_THROW(exception) \
    do { std::cerr << (exception).what() << std::endl; std::abort(); } while (0)
#endif

#else

#define HANDLEGRAPH_THROW(exception) throw (exception)

#endif

#endif
