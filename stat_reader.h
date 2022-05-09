#pragma once
#include <iostream>

#include "transport_catalogue.h"

namespace stat_reader
{
	enum class INQUIRY_TYPE
	{
		ROUTE,
		STOP
	};

	struct Inquiry
	{
		INQUIRY_TYPE type;
		std::string name;
	};

	void ProcessInquiresFromStream(std::istream& is, std::ostream& os, TransportCatalogue& catalogue);

	void PrintRouteInfo(std::ostream& os, RouteInfo& route_info);

	void PrintStopInfo(std::ostream& os, StopInfo& stop_info);

	void ProcessInquiry(std::ostream& os, Inquiry& inq, TransportCatalogue& catalogue);
}