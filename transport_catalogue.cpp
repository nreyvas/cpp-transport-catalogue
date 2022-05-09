#include "transport_catalogue.h"
#include "set"


void TransportCatalogue::AddRoute(std::string name, std::vector<std::string> stop_names, bool IsCircular)
{
	if (routename_to_route_.count(name)) return;

	Route new_route;
	new_route.stops = StopNamesToStopPointers(stop_names);
	new_route.name = std::move(name);
	new_route.is_circular = IsCircular;
	auto& route_ref = routes_database_.emplace_back(std::move(new_route));
	routename_to_route_.insert({ std::string_view(route_ref.name), &route_ref });

	// Updating routes_to_stop_
	for (const Stop* stop : route_ref.stops)
	{
		routes_at_stop_[stop->name].insert(&route_ref);
	}
}

void TransportCatalogue::AddStop(std::string name, Coordinates coordinates)
{
	if (stopname_to_stop_.count(name)) return;
	auto& stop_ref = stops_database_.emplace_back(Stop{ std::move(name), coordinates });
	stopname_to_stop_.insert({ std::string_view(stop_ref.name), &stop_ref });
}


const Route* TransportCatalogue::GetRouteByName(std::string name)
{
	if (routename_to_route_.count(name))
	{
		return routename_to_route_.at(name);
	}
	return nullptr;
}

const Stop* TransportCatalogue::GetStopByName(std::string name)
{
	if (stopname_to_stop_.count(name))
	{
		return stopname_to_stop_.at(name);
	}
	return nullptr;
}

RouteInfo TransportCatalogue::GetRouteInfo(std::string name) const
{
	if (routename_to_route_.count(name) == 0)
		return RouteInfo{false, name, 0, 0, 0.0};
	const Route* route_ptr = routename_to_route_.at(name);
	RouteInfo result;
	result.IsFound = true;
	result.name = std::move(name);
	result.stop_amount = route_ptr->is_circular ? route_ptr->stops.size() : route_ptr->stops.size() * 2 - 1;
	result.unique_stop_amount = CalculateUniqueStops(route_ptr);
	result.length = CalculateLength(route_ptr);
	return result;
}

StopInfo TransportCatalogue::GetStopInfo(std::string name) const
{
	if (stopname_to_stop_.count(name) == 0)
	{
		return StopInfo{ false, name, {} };
	}
	StopInfo result{ true, name, {} };
	if (routes_at_stop_.count(name) != 0)
	{
		std::set<const Route*> list_of_routes = routes_at_stop_.at(name);
		for (const Route* route_ptr : list_of_routes)
		{
			result.routes.insert(route_ptr->name);
		}
	}
	return result;
}

std::vector<const Stop*> TransportCatalogue::StopNamesToStopPointers(std::vector<std::string> stop_names)
{
	std::vector<const Stop*> stop_pointers;
	for (std::string& name : stop_names)
	{
		stop_pointers.push_back(stopname_to_stop_.at(name));
	}
	return stop_pointers;
}

int TransportCatalogue::CalculateUniqueStops(const Route* route_ptr) const
{
	std::set<std::string_view> unique_stops;
	for (const Stop* stop_ptr : route_ptr->stops)
	{
		unique_stops.insert(std::string_view(stop_ptr->name));
	}
	return int(unique_stops.size());
}

double TransportCatalogue::CalculateLength(const Route* route_ptr) const
{
	double length = 0.0;
	for (int i = 0; i < route_ptr->stops.size() - 1; ++i)
	{
		length += ComputeDistance(route_ptr->stops[i]->coordinates, route_ptr->stops[i + 1]->coordinates);
	}
	if (!(route_ptr->is_circular))
	{
		for (int i = route_ptr->stops.size() - 1; i > 0; --i)
		{
			length += ComputeDistance(route_ptr->stops[i]->coordinates, route_ptr->stops[i - 1]->coordinates);
		}
	}
	return length;
}

void TransportCatalogue::AddStopToDistances(const Stop* stop_ptr)
{
	if (stopname_to_stop_.count(stop_ptr->name))	// if this stop is present in stopname_to_stop_, everytime check if the iterated stop != stop_ptr
	{
		for (auto& [_, other_stop_ptr] : stopname_to_stop_)	// iterating without checks
		{
			if (other_stop_ptr == stop_ptr) continue;
			distances_.insert({ { stop_ptr, other_stop_ptr },
				ComputeDistance(stop_ptr->coordinates, other_stop_ptr->coordinates) });
			distances_.insert({ { other_stop_ptr, stop_ptr },
				ComputeDistance(other_stop_ptr->coordinates, stop_ptr->coordinates) });
		}
	}
	else // iterating without checks
	{
		for (auto& [_, other_stop_ptr] : stopname_to_stop_)	
		{
			distances_.insert({ { stop_ptr, other_stop_ptr },
				ComputeDistance(stop_ptr->coordinates, other_stop_ptr->coordinates) });
			distances_.insert({ { other_stop_ptr, stop_ptr },
				ComputeDistance(other_stop_ptr->coordinates, stop_ptr->coordinates) });
		}
	}
}