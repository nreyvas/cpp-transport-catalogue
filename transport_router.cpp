#include "transport_router.h"

namespace catalogue {

TransportRouter::TransportRouter(RoutingSettings settings,
                                const TransportCatalogue& cat) 
    : routing_settings_(std::move(settings))
    , cat_(cat) {

    }
    
const RoutingSettings& TransportRouter::GetRoutingSettings() const {
    return routing_settings_;
}

void TransportRouter::SetRoutingSettings(RoutingSettings routing_settings) {
    routing_settings_ = std::move(routing_settings);
}

void TransportRouter::AddStopVertex(const Stop* stop) {
    vertex_index_to_stop_.push_back(stop);
    vertex_index_to_stop_.push_back(stop);
    //Сохраняем только четные ид - приемные остановки, 
    //"исхожящие" вершины можно получить прибавив единицу к ид.
    stopname_to_vertex_id_.emplace(stop->name_, vertex_index_to_stop_.size() - 2);
}

void TransportRouter::AddBusWaitEdges() {
    //edge_index_to_bus_.resize(std::pow(stopname_to_stop_.size(), 3));
    //вызов конструктора графа для известного на данный момент числа вершин графа
    route_graph_ = std::move(graph::DirectedWeightedGraph<BusRouteWeight>(vertex_index_to_stop_.size()));
    for(graph::VertexId vertex_from_id = 0; vertex_from_id < vertex_index_to_stop_.size(); vertex_from_id += 2) {
        route_graph_.AddEdge({vertex_from_id, vertex_from_id + 1, routing_settings_.bus_wait_time});
        edge_index_to_bus_.push_back(nullptr);
    }
}

void TransportRouter::AddBusEdges(std::string_view name) {
    if(cat_.busname_to_bus_.count(name) == 0) {
        throw std::logic_error("No such bus");
    }
    const Bus* bus = cat_.busname_to_bus_.at(name);
    const std::vector<const Stop*>& stops = bus->stops_;

    if(bus->bus_type_ == BusType::CYCLED) {
        AddBusStopsEdges(bus, stops.begin(), stops.end());
    } else {
        AddBusStopsEdges(bus, stops.begin(), stops.end());
        AddBusStopsEdges(bus, stops.rbegin(), stops.rend());
    }
}

graph::VertexId TransportRouter::GetStopVertexIndex(std::string_view stop_name) const {
    if(stopname_to_vertex_id_.count(stop_name) == 0) {
        throw std::logic_error("Invalid stop name - can't find VertexId");
    }
    return stopname_to_vertex_id_.at(stop_name);
}

const Bus* TransportRouter::GetBusByEdgeIndex(graph::EdgeId edge_id) const {
    if(edge_id < edge_index_to_bus_.size()) {
        return edge_index_to_bus_[edge_id];
    } else {
        throw std::logic_error("Bad EdgeId requested!");
    }
}

const graph::Edge<BusRouteWeight>& TransportRouter::GetEdgeByIndex(graph::EdgeId edge_id) const {
    return route_graph_.GetEdge(edge_id);
}

const Stop* TransportRouter::GetStopByVertexIndex(graph::VertexId vertex_id) const {
    if (vertex_id >= vertex_index_to_stop_.size()) {
        throw std::logic_error("Bad vartex id is passed, too big");
    }
    return vertex_index_to_stop_[vertex_id];
}

const std::deque<const Stop*>& TransportRouter::GetVertexIndexToStop() const {
    return vertex_index_to_stop_;
}

const std::deque<const Bus*>& TransportRouter::GetEdgeIndexToBus() const {
    return edge_index_to_bus_;
}

void TransportRouter::SetRouteGraph(graph::DirectedWeightedGraph<BusRouteWeight>&& route_graph) {
    route_graph_ = route_graph;
}
void TransportRouter::SetVertexIndexToStop(std::deque<const Stop*>&& vertex_index_to_stop) {
    vertex_index_to_stop_ = vertex_index_to_stop;
}
void TransportRouter::SetEdgeIndexToBus(std::deque<const Bus*>&& edge_index_to_bus) {
    edge_index_to_bus_ = edge_index_to_bus;
}
void TransportRouter::SetStopnameToVertexId(std::map<std::string_view, graph::VertexId>&& stopname_to_vertex_id) {
    stopname_to_vertex_id_ = stopname_to_vertex_id;
}
} // namespace catalogue