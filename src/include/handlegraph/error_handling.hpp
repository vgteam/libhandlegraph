#ifndef HANDLEGRAPH_ERROR_HANDLING_HPP_INCLUDED
#define HANDLEGRAPH_ERROR_HANDLING_HPP_INCLUDED

/** \file
 * Defines HANDLEGRAPH_THROW, the macro libhandlegraph uses to report
 * unrecoverable errors.
 */

#include <stdexcept>

#define HANDLEGRAPH_THROW(exception) throw (exception)

#endif
