#pragma once

#include "ranges.h"

#include <cstdlib>
#include <map>
#include <vector>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

struct EdgeData {
    size_t bus_id = 0;
    size_t stop_id = 0;
    int span_count = 0;
    double weight_bus = 0.;
    double bus_wait_time = 0.;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;
    explicit DirectedWeightedGraph(size_t vertex_count);
    explicit DirectedWeightedGraph(std::vector<Edge<Weight>>&& edges, std::vector<IncidenceList>&& incidence_lists, std::map<EdgeId, EdgeData>&& edgeid_to_params) :
    	edges_(std::move(edges)), incidence_lists_(std::move(incidence_lists)), edgeid_to_params_(std::move(edgeid_to_params)) {}
    EdgeId AddEdge(const Edge<Weight>& edge);

    size_t GetVertexCount() const;
    size_t GetEdgeCount() const;
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    const std::vector<Edge<Weight>>& GetEdges() const {
    	return edges_;
    }
    const std::vector<IncidenceList>& GetIncidenceLists() const {
    	return incidence_lists_;
    }
    std::map<EdgeId, EdgeData>& SetParams() {
    	return edgeid_to_params_;
    }
    const std::map<EdgeId, EdgeData>& GetParams() const {
    	return edgeid_to_params_;
    }

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
    std::map<EdgeId, EdgeData> edgeid_to_params_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph
