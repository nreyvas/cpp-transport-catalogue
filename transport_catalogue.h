#pragma once

#include <deque>
#include <iostream>
#include <string_view>
#include <unordered_map>

#include "domain.h"

using namespace std::string_literals;

namespace transport_catalogue
{
	class TransportCatalogue
	{
	private:

		std::deque<Stop> stops_database_;
		std::deque<Bus> routes_database_;
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, const Bus*> routename_to_route_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHash> distances_;
		std::unordered_map<std::string_view, std::set<const Bus*>> routes_at_stop_;

	public:

		void AddRoute(std::string name, std::vector<std::string> stop_names, bool IsCircular);
		void AddStop(std::string name, geo::Coordinates coordinates);
		void AddDistance(std::string name_from, std::string name_to, int distance);
		const Bus* GetBusByName(std::string& name);
		const Stop* GetStopByName(std::string& name);
		BusInfo GetBusInfo(std::string& name) const;
		StopInfo GetStopInfo(std::string& name) const;
		int GetDistance(std::string name_from, std::string name_to) const;
		std::vector<const Bus*> GetBuses() const;
		std::vector<const Stop*> GetActiveStops() const;
		const std::deque<Stop>& GetAllStops() const;
		const std::deque<Bus>& GetAllBuses() const;
		const std::vector<std::pair<int,int>> GetBusDistances(const Bus& bus);

	private:

		std::vector<const Stop*> StopNamesToStopPointers(std::vector<std::string> stop_names);
		int CalculateUniqueStops(const Bus* route_ptr) const;
		int CalculateLength(const Bus* route_ptr) const;
		double CalculateCurvature(const Bus* route_ptr) const;
	};
}