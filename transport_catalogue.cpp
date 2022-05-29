#include "transport_catalogue.h"

#include <algorithm>

namespace transport_catalogue
{
	void TransportCatalogue::AddRoute(std::string name, std::vector<std::string> stop_names, bool IsCircular)
	{
		if (routename_to_route_.count(name)) return;

		Bus new_route;
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

	void TransportCatalogue::AddStop(std::string name, geo::Coordinates coordinates)
	{
		if (stopname_to_stop_.count(name)) return;
		auto& stop_ref = stops_database_.emplace_back(Stop{ name, coordinates });
		stopname_to_stop_.insert({ std::string_view(stop_ref.name), &stop_ref });
	}

	void TransportCatalogue::AddDistance(std::string name_from, std::string name_to, int distance)
	{
		const Stop* stop_from_ptr = stopname_to_stop_.at(name_from);
		const Stop* stop_to_ptr = stopname_to_stop_.at(name_to);
		distances_.insert({ { stop_from_ptr, stop_to_ptr }, distance });
	}

	const Bus* TransportCatalogue::GetBusByName(std::string& name)
	{
		if (routename_to_route_.count(name))
		{
			return routename_to_route_.at(name);
		}
		return nullptr;
	}

	const Stop* TransportCatalogue::GetStopByName(std::string& name)
	{
		if (stopname_to_stop_.count(name))
		{
			return stopname_to_stop_.at(name);
		}
		return nullptr;
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string& name) const
	{
		if (routename_to_route_.count(name) == 0)
		{
			return BusInfo{ 0, name, false };
		}
		const Bus* route_ptr = routename_to_route_.at(name);
		BusInfo result{0, name, true};
		result.stop_count = route_ptr->is_circular ? route_ptr->stops.size() : route_ptr->stops.size() * 2 - 1;
		result.unique_stop_count = CalculateUniqueStops(route_ptr);
		result.length = CalculateLength(route_ptr);
		result.curvature = CalculateCurvature(route_ptr);
		return result;
	}

	StopInfo TransportCatalogue::GetStopInfo(std::string& name) const
	{
		if (stopname_to_stop_.count(name) == 0)
		{
			return StopInfo{ 0, name, false};
		}
		StopInfo result{ 0, name, true };

		if (routes_at_stop_.count(name) != 0)
		{
			result.routes = routes_at_stop_.at(name);
		}

		return result;
	}

	int TransportCatalogue::GetDistance(std::string name_from, std::string name_to) const
	{
		const Stop* stop_from_ptr = stopname_to_stop_.at(name_from);
		const Stop* stop_to_ptr = stopname_to_stop_.at(name_to);
		return distances_.at({ stop_from_ptr, stop_to_ptr });
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

	std::vector<const Bus*> TransportCatalogue::GetBuses() const
	{
		std::vector<const Bus*> result;
		result.reserve(routes_database_.size());
		for (const Bus& bus : routes_database_)
		{
			result.push_back(&bus);
		}
		std::sort(result.begin(), result.end(), [](const Bus* lhs, const Bus* rhs)
			{ return lhs->name < rhs->name; });
		return result;
	}

	std::vector<const Stop*> TransportCatalogue::GetActiveStops() const
	{
		std::set<const Stop*> set_result;
		for (const Bus& bus : routes_database_)
		{
			for (const Stop* stop_ptr : bus.stops)
			{
				//result.push_back(stop_ptr);
				set_result.insert(stop_ptr);
			}
		}
		std::vector<const Stop*> result{ set_result.begin(), set_result.end() };
		std::sort(result.begin(), result.end(), [](const Stop* lhs, const Stop* rhs)
			{ return lhs->name < rhs->name;	});
		return result;
	}

	int TransportCatalogue::CalculateUniqueStops(const Bus* route_ptr) const
	{
		std::set<std::string_view> unique_stops;
		for (const Stop* stop_ptr : route_ptr->stops)
		{
			unique_stops.insert(std::string_view(stop_ptr->name));
		}
		return int(unique_stops.size());
	}

	int TransportCatalogue::CalculateLength(const Bus* route_ptr) const
	{
		double length = 0.0;
		for (size_t i = 0; i < route_ptr->stops.size() - 1; ++i)
		{
			if (distances_.count({ route_ptr->stops[i], route_ptr->stops[i + 1] }) != 0)
			{
				length += distances_.at({ route_ptr->stops[i], route_ptr->stops[i + 1] });
			}
			else
			{
				length += distances_.at({ route_ptr->stops[i + 1], route_ptr->stops[i] });
			}
		}
		if (!(route_ptr->is_circular))
		{
			for (int i = route_ptr->stops.size() - 1; i > 0; --i)
			{
				if (distances_.count({ route_ptr->stops[i], route_ptr->stops[i - 1] }) != 0)
				{
					length += distances_.at({ route_ptr->stops[i], route_ptr->stops[i - 1] });
				}
				else
				{
					length += distances_.at({ route_ptr->stops[i - 1], route_ptr->stops[i] });
				}
			}
		}
		return length;
	}

	double TransportCatalogue::CalculateCurvature(const Bus* route_ptr) const
	{
		double geo_length = 0.0;
		for (size_t i = 0; i < route_ptr->stops.size() - 1; ++i)
		{
			geo_length += ComputeDistance(route_ptr->stops[i]->coordinates, route_ptr->stops[i + 1]->coordinates);
		}
		if (!(route_ptr->is_circular))
		{
			for (int i = route_ptr->stops.size() - 1; i > 0; --i)
			{
				geo_length += ComputeDistance(route_ptr->stops[i]->coordinates, route_ptr->stops[i - 1]->coordinates);
			}
		}
		return CalculateLength(route_ptr) / geo_length;
	}
}