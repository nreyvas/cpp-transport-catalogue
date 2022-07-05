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
		MAP,
		ROUTE
    };

	//--------------------------------------------------------------

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

	struct RouteInfo : Info
	{
		enum class ElementType
		{
			WAIT,
			BUS
		};
		struct RouteElement
		{
			ElementType type;
			std::string_view stop_name;
			std::string_view bus_name;
			double time = 0.0;
			int span_count;
		};

		double total_time;
		std::vector<RouteElement> route_elements;
	};

	//--------------------------------------------------------------
	
	struct StopPairHash
	{
		size_t operator() (const std::pair<const Stop*, const Stop*> p) const;
		std::hash<std::string> hasher_;
	};

	struct Span
	{
		double distance = 0;
		std::string_view bus_name;
		int inner_spans_amount = 0;
	};

	Span operator+(const Span& lhs, const Span& rhs);

	bool operator<(const Span& lhs, const Span& rhs);

	bool operator>(const Span& lhs, const Span& rhs);
}