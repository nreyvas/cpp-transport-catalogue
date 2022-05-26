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
		Coordinates coordinates;
	};
	
	struct Route
	{
		std::string name;
		std::vector<const Stop*> stops;
		bool is_circular = false;
	};

	enum class RequestType
    {
        STOP,
        BUS
    };

	struct Info
	{
		Info() = default;
		Info(int no, std::string n, RequestType t, bool found);
		int id = 0;
		std::string name;
		RequestType type;
		bool IsFound = false;
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
		std::set<const Route*> routes;
	};
	
	struct StopPairHash
	{
		size_t operator() (const std::pair<const Stop*, const Stop*> p) const;
		std::hash<std::string> hasher_;
	};
}