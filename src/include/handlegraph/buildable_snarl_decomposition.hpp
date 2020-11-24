#ifndef HANDLEGRAPH_BUILDABLE_SNARL_DECOMPOSITION_HPP_INCLUDED
#define HANDLEGRAPH_BUILDABLE_SNARL_DECOMPOSITION_HPP_INCLUDED

/** \file 
 * Defines the base SnarlDecomposition interface.
 */

#include "handlegraph/snarl_decomposition.hpp"

namespace handlegraph {

/*
 * Defines an interface for storing a decomposition of a graph into snarls.
 */
class BuildableSnarlDecomposition : public SnarlDecomposition {
    
public:

    virtual ~BuildableSnarlDecomposition() = default;
    
    /**
     * Represents a source of snarl decomposition information.
     *
     * A matching function will call begin_chain and end_chain, and begin_snarl
     * and end_snarl, for each chain and snarl in the snarl decomposition,
     * accroding to the nesting structure of the decomposition.
     *
     * The decomposition is implicitly rooted at the root snarl, so the series
     * of calls will begin with a chain, if the graph is nonempty.
     *
     * Calls will be made for empty snarls, and for trivial chains of single
     * nodes.
     *
     * Trivial chains and circular chains are distinguished by circular chains
     * having contents.
     *
     */
    using decomposition_source_t = std::function<void(const std::function<void(const handle_t&)>& begin_chain, const std::function<void(const handle_t&)>& end_chain,
                                                      const std::function<void(const handle_t&)>& begin_snarl, const std::function<void(const handle_t&)>& end_snarl)>;
    
    /**
     * Calls the given decomposition-traversing function, and gives it
     * callbacks which it can call to enter and exit chains and snarls.
     *
     * Will fill in this SnarlDecomposition with the snarls that are produced.
     *
     * The SnarlDecomposition will need to know connectivity through snarls and
     * chains, and whether tip-to-tip traversals are realizable, so it will be
     * difficult to implement this method without having access at the class
     * level to the backing HandleGraph. To facilitate computing connectivity,
     * it may be wise to guarantee that connectivity queries will work for all
     * snarls and chains that have already been closed, while this function is
     * running.
     *
     * There is no built-in parallel construction. The decomposition source can
     * compute the stream of begin and end calls in parallel and then linearize
     * it.
     */
    virtual void build_snarl_decomposition(const decomposition_source_t& traverse_decomposition) = 0;
   
};

}

#endif
