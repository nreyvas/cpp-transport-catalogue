#include "transport_router.h"


namespace router
{
	void TransportRouter::SetBusWaitTime(int time)
	{
		bus_wait_time_ = time;
	}

	void TransportRouter::SetBusVelocity(double vel)
	{
		bus_velocity_ = vel;
	}

	int TransportRouter::GetBusWaitTime() const
	{
		return bus_wait_time_;
	}

	double TransportRouter::GetBusVelocity() const
	{
		return bus_velocity_;
	}

	void TransportRouter::AddStop(const transport_catalogue::Stop& stop)
	{
		id_to_stop_.insert({ stop_counter_, &stop });
		stop_to_id_.insert({ &stop, stop_counter_ });
		++stop_counter_;
	}

	void TransportRouter::AddBus(const transport_catalogue::Bus& bus)
	{
		std::vector<std::pair<int,int>> bus_distances = catalogue_.GetBusDistances(bus);
		std::vector<const transport_catalogue::Stop*> stops = bus.stops;

		for (size_t stop_from_number = 0; stop_from_number < stops.size(); ++stop_from_number)
		{
			for (size_t stop_to_number = 0; stop_to_number < stops.size(); ++stop_to_number)
			{
				if (stop_from_number == stop_to_number)
				{
					continue;
				}
				if (bus.is_circular && stop_from_number > stop_to_number)
				{
					continue;
				}
				size_t stop_from_id = stop_to_id_.at(stops[stop_from_number]);
				size_t stop_to_id = stop_to_id_.at(stops[stop_to_number]);
				int total_distance = 0;
				int span_counter = std::abs((int)stop_from_number - (int)stop_to_number);
				if (stop_from_number < stop_to_number)
				{
					for (int i = stop_from_number; i != stop_to_number; ++i)
					{
						total_distance += bus_distances[i].first;
					}
				}
				else
				{
					for (int i = stop_from_number; i != stop_to_number; --i)
					{
						total_distance += bus_distances[i - 1].second;
					}
				}
				double waiting_distance = bus_wait_time_ * (bus_velocity_ * 1000 / 60);	// NEW
				transport_catalogue::Span span{ total_distance + waiting_distance, bus.name, span_counter };	// NEW
				graph::Edge<transport_catalogue::Span> edge{ stop_from_id, stop_to_id, span};
				graph_.AddEdge(edge);
			}
		}
	}

	RouteInfo TransportRouter::GetRouteInfo(const Stop* from_stop, const Stop* to_stop) const
	{
		RouteInfo result;
		auto route_info = router_->BuildRoute(stop_to_id_.at(from_stop), stop_to_id_.at(to_stop));
		if (!route_info)
		{
			return result;
		}
		result.type = RequestType::ROUTE;
		result.is_found = true;
		
		result.total_time = 0;
		result.route_elements.reserve(route_info->edges.size() * 2);	// each edge has wait + travel
		for (const graph::EdgeId edge_id : route_info->edges)
		{
			const graph::Edge<Span> edge = graph_.GetEdge(edge_id);

			RouteInfo::RouteElement wait_element;
			wait_element.type = RouteInfo::ElementType::WAIT;
			wait_element.stop_name = id_to_stop_.at(edge.from)->name;
			wait_element.time = bus_wait_time_;
			result.total_time += bus_wait_time_;
			result.route_elements.push_back(wait_element);

			RouteInfo::RouteElement bus_element;
			bus_element.type = RouteInfo::ElementType::BUS;
			bus_element.bus_name = edge.weight.bus_name;
			bus_element.span_count = edge.weight.inner_spans_amount;
			bus_element.time = edge.weight.distance / (bus_velocity_ * 1000 / 60.0) - bus_wait_time_;
			result.total_time += bus_element.time;
			result.route_elements.push_back(bus_element);
		}
		return result;
	}

	void TransportRouter::Activate()
	{
		router_ = new graph::Router<Span>{ graph_ };
	}
}