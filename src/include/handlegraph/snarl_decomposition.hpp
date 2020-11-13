#ifndef HANDLEGRAPH_SNARL_DECOMPOSITION_HPP_INCLUDED
#define HANDLEGRAPH_SNARL_DECOMPOSITION_HPP_INCLUDED

/** \file 
 * Defines the base SerializableHandleGraph interface.
 */

#include <iostream>
#include <fstream>
#include <arpa/inet.h>

namespace handlegraph {

/*
 * Defines an interface for decompositions of graphs into snarls and chains.
 * The decomposition is rooted with a root snarl, which has undefined bounding nodes.
 * Within each snarl are 0 or more chains (which act like nodes).
 * Within each chain is one or more nodes, and, between nodes, there are snarls.
 *
 * This interface does not include any net graph functionality, for traversing inside a snarl while stepping over chains.
 * For that, see NetGraphSnarlDecomposition. TODO: write that.
 */
class SnarlDecomposition {
    
public:

    virtual ~SnarlDecomposition() = default;
    
    /**
     * Get a net handle referring to a tip-to-tip traversal of the contents of the root snarl.
     * TODO: Special handling for circular things in the root snarl? Circular traversal type?
     */
    virtual bool get_root(const net_handle_t& net) const = 0;
    
    
    /**
     * Return true if the given handle refers to (a traversal of) the root
     * snarl, and false otherwise.
     */
    virtual bool is_root(const net_handle_t& net) const = 0;
    
    // TODO: Are these 1-hot encodings a good API?

    /**
     * Returns true if the given net handle refers to (a traversal of) a snarl.
     */
    virtual bool is_snarl(const net_handle_t& net) const = 0;
    /**
     * Returns true if the given net handle refers to (a traversal of) a chain.
     */
    virtual bool is_chain(const net_handle_t& net) const = 0;
    /**
     * Returns true if the given net handle refers to (a traversal of) a single node, and thus has a corresponding handle_t.
     */
    virtual bool is_node(const net_handle_t& net) const = 0;
    /**
     * Return true if the given net handle is a snarl bound sentinel (in either
     * inward or outward orientation), and false otherwise.
     */
    virtual bool is_sentinel(const net_handle_t& net) const = 0;
    
    // We have sentinel net_handle_t values for the start/end of each snarl, so
    // that we can tell which last edge a traversal of the contents of a snarl
    // takes when we represent it as a list of net_handle_t items. We also use
    // these to query what's attached to the snarl start/end when traversing,
    // and to see self loops immediately inside the snarl. These may actually
    // just be the handles for the nodes at the start'end of the snarl with
    // special flags set.
    //
    // For chains, we use the net handles to the appropriate first/last nodes
    // in the appropriate orientation.
    
    /**
     * Get the bounding handle for the snarl or chain referenced by the given
     * net handle, getting the start or end facing in or out as appropriate.
     *
     * For snarls, returns the bounding sentinel net handles. For chains,
     * returns net handles for traversals of the bounding nodes of the chain.
     *
     * May not be called on traversals of individual nodes.
     */
    virtual net_handle_t get_bound(const net_handle_t& snarl, bool get_end, bool face_in) const = 0;
    
    /**
     * Return true if the given net handle describes a category of traversal that starts at the local start of the snarl/chain/node.
     */
    virtual bool starts_at_start(const net_handle_t& net) const = 0;
    /**
     * Return true if the given net handle describes a category of traversal that starts at the local end of the snarl/chain/node.
     */
    virtual bool starts_at_end(const net_handle_t& net) const = 0;
    /**
     * Return true if the given net handle describes a category of traversal that starts at an internal tip in the snarl/chain. Never true for nodes.
     */
    virtual bool starts_at_tip(const net_handle_t& net) const = 0;
    
    /**
     * Return true if the given net handle describes a category of traversal that ends at the local start of the snarl/chain/node.
     */
    virtual bool ends_at_start(const net_handle_t& net) const = 0;
    /**
     * Return true if the given net handle describes a category of traversal that ends at the local end of the snarl/chain/node.
     */
    virtual bool ends_at_end(const net_handle_t& net) const = 0;
    /**
     * Return true if the given net handle describes a category of traversal that ends at the an internal tip in the snarl/chain. Never true for nodes.
     */
    virtual bool ends_at_tip(const net_handle_t& net) const = 0;
    
    /**
     * Return a net handle to the same snarl/chain/node in the opposite orientation.
     * No effect on tip-to-tip, start-to-start, or end-to-end net handles. Flips all the others.
     */
    virtual net_handle_t flip(const net_handle_t& net) const = 0;
    
    /**
     * Get a start-to-end traversal handle from any net handle.
     * Mostly useful to normalize for equality comparisons; the traversal may not be possible given internal connectivity.
     */
    virtual net_handle_t start_to_end(const net_handle_t& net) const = 0;
    
    /**
     * Turn a handle to an oriented node into a net handle for a start-to-end or end-to-start traversal of the node, as appropriate.
     */
    virtual net_handle_t get_net(const handle_t& handle) const = 0;
    
    /**
     * For a net handle to a traversal of a single node, get the handle for that node in the orientation it is traversed.
     * May not be called for other net handles.
     */
    virtual handle_t get_handle(const net_handle_t& net) const = 0;
    
    /**
     * Get the parent snarl of a chain, or the parent chain of a snarl or node.
     * If the child is start-to-end or end-to-start, and the parent is a chain,
     * the chain comes out facing the same way, accounting for the relative
     * orientation of the child snarl or node in the chain. Otherwise,
     * everything is produced as start-to-end, even if that is not actually a
     * realizable traversal of a snarl or chain. May not be called on the root
     * snarl.
     *
     * Also works on snarl boundary sentinels.
     */
    virtual net_handle_t get_parent(const net_handle_t& child) const = 0;
    
    /**
     * Get a net handle for traversals of a the snarl or chain that contains
     * the given oriented bounding node traversals or sentinels. Given two
     * sentinels for a snarl, produces a net handle to a start-to-end,
     * end-to-end, end-to-start, or start-to-start traversal of that snarl.
     * Given handles to traversals of the bounding nodes of a chain, similarly
     * produces a net handle to a traversal of the chain.
     *
     * For a chain, either or both handles can also be a snarl containing tips,
     * for a tip-to-start, tip-to-end, start-to-tip, end-to-tip, or tip-to-tip
     * traversal. Similarly, for a snarl, either or both handles can be a chain
     * in the snarl that contains internal tips, or that has no edges on the
     * appropriate end.
     */
    virtual net_handle_t get_parent_traversal(const net_handle_t& traversal_start, const net_handle_t& traversal_end) const = 0;
    
    
    /**
     * Get a handle to the inward-facing traversal of the first node in a chain
     * or the start boundary in a snarl.
     *
     * This isn't necessarily where the traversal specified by the given handle
     * actually starts (it may be end to end or tip to tip, for example.)
     */
    virtual net_handle_t get_start(const net_handle_t& parent) const = 0;
    
    /**
     * Get a handle to the outward-facing traversal of the last node in a chain
     * or the end boundary in a snarl.
     *
     * This isn't necessarily where the traversal specified by the given handle
     * actually ends (it may be start to start or tip to tip, for example.)
     */
    virtual net_handle_t get_end(const net_handle_t& parent) const = 0;
    
    // TODO: How to find all the tips that can actually start traversals?
    // Maybe just find all of them.
    // Want to track whether tips are reachable from the start and the end separately, possibly.
    // TODO: Function that gets you where exactly the kind of traversal you are looking at starts and ends.
        // Would need to handle tips, and not be required to filter tips for validity
        // Except that when producing child snarls/chains containing tips as tip options it needs to know that e.g. tip-to-start traversals are realizable for that child before producing it.
            // Probably worth precomputing
    
    template<typename Iteratee>
    bool for_each_traversal_start(const net_handle_t& traversal, const Iteratee&) const = 0;
    
    template<typename Iteratee>
    bool for_each_traversal_end(const net_handle_t& traversal, const Iteratee&) const = 0;
    
    /**
     * Loop over the child snarls and nodes of a chain, or the child chains of
     * a snarl. If the parent is a chain and is in start-to-end or end-to-start
     * order, children are porduced in the order and orientation they are
     * encountered when viewing the chain that way. Otherwise, children are
     * produced in arbitrary order and start-to-end orientation, even if that
     * is not actually a realizable traversal of a snarl or chain.
     *
     * Return false from the iteratee to stop, or void or true to keep going.
     * Returns false if iteration was stopped early, and true otherwise.
     */
    template<typename Iteratee>
    bool for_each_child(const net_handle_t& parent, const Iteratee& iteratee) const = 0;
    
    /**
     * Given a net handle to any type of traversal of a snarl, chain, or node,
     * loop over only the types of traversals that are possible. Produces a net
     * handle to the same snarl, chain, or node for each possible combination
     * of traversal starts and ends (start, end, or internal tip) that is
     * permitted by the internal connectivity of the snarl, chain, or node.
     *
     * Return false from the iteratee to stop, or void or true to keep going.
     * Returns false if iteration was stopped early, and true otherwise.
     */
    template<typename Iteratee>
    bool for_each_traversal(const net_handle_t& any, const Iteratee& iteratee) const;
    
    /**
     * Given a net handle to a traversal of a snarl, chain, or node, iterate
     * over all kinds of traversals of all snarls, chains, or nodes that are
     * reachable by going either left or right from the given traversal.
     *
     * Does not check to see if the given traversal is possible given the
     * internal connectivity of a snarl or chain being traversed.
     *
     * Does not leave a chain; looking form a node traversal out the end of its
     * chain will produce nothing.
     *
     * Produces and accepts snarl start and end sentinels, and does not leave a
     * snarl.
     *
     * If you do want to leave a snarl or a chain, jump up to the parent with get_parent_traversal().
     */
    template<typename Iteratee>
    bool follow_net_edges(const net_handle_t& here, bool go_left, const Iteratee& iteratee) const = 0;
    
    /**
     * Like follow_net_edges, except it only accepts start-to-end and
     * end-to-start handles, only produces start-to-end and end-to-start
     * handles, and pretends all child snarls and chains are connected only
     * across, with no hairpins or internal tips.
     */
    template<typename Iteratee>
    bool follow_net_edges_ignoring_internal_connectivity(const net_handle_t& here, bool go_left, const Iteratee& iteratee) const = 0;
    
   
protected:

    // TODO: _impls for each iterator
    
};





/**
 * Inline implementations
 */
        
}

#endif
