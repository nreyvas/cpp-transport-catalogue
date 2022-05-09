#pragma once

#include <deque>
#include <iostream>
#include <string_view>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "geo.h"

using namespace std::string_literals;

struct Stop
{
	std::string name;
	Coordinates coordinates;
};

struct Route
{
	std::string name;
	std::vector<const Stop*> stops;
	bool is_circular = false;
};

struct RouteInfo
{
	bool IsFound = false;
	std::string name;
	int stop_amount;
	int unique_stop_amount;
	double length;
};

struct StopInfo
{
	bool IsFound = false;
	std::string name;
	std::set<std::string_view> routes;
};

struct StopPairHash
{
	size_t operator() (const std::pair<const Stop*, const Stop*> p) const
	{
		return hasher_(p.first->name) * 37 + hasher_(p.second->name);
	}
	std::hash<std::string> hasher_;
};

class TransportCatalogue
{
private:
	std::deque<Stop> stops_database_;
	std::deque<Route> routes_database_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, const Route*> routename_to_route_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> distances_;
	std::unordered_map<std::string_view, std::set<const Route*>> routes_at_stop_;
public:
	void AddRoute(std::string name, std::vector<std::string> stop_names, bool IsCircular);
	void AddStop(std::string name, Coordinates coordinates);
	const Route* GetRouteByName(std::string name);
	const Stop* GetStopByName(std::string name);
	RouteInfo GetRouteInfo(std::string name) const;
	StopInfo GetStopInfo(std::string name) const;
private:
	std::vector<const Stop*> StopNamesToStopPointers(std::vector<std::string> stop_names);
	int CalculateUniqueStops(const Route* route_ptr) const;
	double CalculateLength(const Route* route_ptr) const;
	void AddStopToDistances(const Stop* stop_ptr);
};