#pragma once
#include <string>
#include <vector>
#include <set>

#include "geo.h"

namespace transport_catalogue
{
	struct Stop
	{
		std::string name;
		geo::Coordinates coordinates;
	};
	
	struct Bus
	{
		std::string name;
		std::vector<const Stop*> stops;
		bool is_circular = false;
	};

	enum class RequestType
    {
        STOP,
        BUS,
		MAP
    };

	struct Info
	{
		Info() = default;
		Info(int no, std::string n, RequestType t, bool found);
		int id = 0;
		std::string name;
		RequestType type;
		bool is_found = false;
	};
	
	struct BusInfo : Info
	{
		BusInfo() = default;
		BusInfo(int no, std::string n, bool found);
		int stop_count;
		int unique_stop_count;
		double length;
		double curvature;
	};
	
	struct StopInfo : Info
	{
		StopInfo() = default;
		StopInfo(int no, std::string n, bool found);
		std::set<const Bus*> routes;
	};

	struct MapInfo : Info
	{
		std::string svg_code;
	};
	
	struct StopPairHash
	{
		size_t operator() (const std::pair<const Stop*, const Stop*> p) const;
		std::hash<std::string> hasher_;
	};
}