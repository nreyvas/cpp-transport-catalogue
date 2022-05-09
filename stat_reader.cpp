#include <string>
#include <vector>
#include <iomanip>

#include "stat_reader.h"

namespace stat_reader
{
	void ProcessInquiresFromStream(std::istream& is, std::ostream& os, TransportCatalogue& catalogue)
	{
		int inquiry_amount;
		is >> inquiry_amount;
		is.ignore(32767, '\n');

		std::vector<Inquiry> inquiries;
		inquiries.reserve(inquiry_amount);
		std::string line;
		for (int i = 0; i < inquiry_amount; ++i)
		{
			Inquiry inquiry;
			std::getline(is, line);
			if (line[0] == 'S')
			{
				inquiry.type = INQUIRY_TYPE::STOP;
			}
			else if (line[0] == 'B')
			{
				inquiry.type = INQUIRY_TYPE::ROUTE;
			}
			line.erase(0, line.find(' ') + 1);
			inquiry.name = std::move(line);
			inquiries.push_back(std::move(inquiry));
			line.clear();
		}
		
		for (Inquiry& inq : inquiries)
		{
			ProcessInquiry(os, inq, catalogue);
		}
	}

	void PrintRouteInfo(std::ostream& os, RouteInfo& route_info)
	{
		if (!route_info.IsFound)
		{
			os << "Bus " << route_info.name << ": not found\n";
		}
		else
		{
			os << "Bus " << route_info.name << ": " <<
				route_info.stop_amount << " stops on route, " <<
				route_info.unique_stop_amount << " unique stops, " <<
				std::setprecision(6) << route_info.length << " route length, " <<
				route_info.curvature << " curvature\n";
		}
	}

	void PrintStopInfo(std::ostream& os, StopInfo& stop_info)
	{
		if (!stop_info.IsFound)
		{
			os << "Stop " << stop_info.name << ": not found\n";
		}
		else if (stop_info.routes.empty())
		{
			os << "Stop " << stop_info.name << ": no buses\n";
		}
		else
		{
			os << "Stop " << stop_info.name << ": buses";
			for (const std::string_view& route_name : stop_info.routes)
			{
				os << " " << route_name;
			}
			os << '\n';
		}
	}

	void ProcessInquiry(std::ostream& os, Inquiry& inq, TransportCatalogue& catalogue)
	{
		switch (inq.type)
		{
		case INQUIRY_TYPE::ROUTE:
		{
			RouteInfo route_info = catalogue.GetRouteInfo(inq.name);
			PrintRouteInfo(os, route_info);
			break;
		}
		case INQUIRY_TYPE::STOP:
		{
			StopInfo stop_info = catalogue.GetStopInfo(inq.name);
			PrintStopInfo(os, stop_info);
			break;
		}
		default:
			break;
		}
	}
}