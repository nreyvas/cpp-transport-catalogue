#include <algorithm>

#include "request_handler.h"


namespace transport_catalogue
{
	BaseRequest::BaseRequest(RequestType t, std::string n)
		: type(t),
		name(n) {}


	BaseStopRequest::BaseStopRequest
	(RequestType t, std::string n, Coordinates c, std::map<std::string, int> rd)
		: BaseRequest(t, n),
		coords(c),
		road_distances(rd) {}

	BaseBusRequest::BaseBusRequest
	(RequestType t, std::string n, std::vector<std::string> s, bool ir)
		: BaseRequest(t, n),
		stops(s),
		is_roundtrip(ir) {}

	//---------------------------------------------------------------------------------------

	void BaseRequestHandler::AddStop(BaseStopRequest& inquiry)	//TODO
	{
		catalogue_.AddStop(inquiry.name, inquiry.coords);
	}

	void BaseRequestHandler::AddDistance(std::string stop_from, std::string stop_to, int distance)
	{
		catalogue_.AddDistance(stop_from, stop_to, distance);
	}

	void BaseRequestHandler::AddBus(BaseBusRequest& inquiry)	//TODO
	{
		catalogue_.AddRoute(std::move(inquiry.name), std::move(inquiry.stops), inquiry.is_roundtrip);
	}

	void ProcessBaseRequests(std::vector<std::unique_ptr<BaseRequest>> requests,
		BaseRequestHandler handler)
	{

		std::sort(requests.begin(), requests.end(), [](const auto& lhs, const auto& rhs)
			{ return lhs->type < rhs->type; });	// sorting requests so the stops are processed before buses

		for (auto& request_ptr : requests)	// adding stops and buses
		{
			if (request_ptr->type == RequestType::STOP)
			{
				auto& request_ref = static_cast<BaseStopRequest&>(*request_ptr);
				handler.AddStop(request_ref);
			}
			else if (request_ptr->type == RequestType::BUS)
			{
				auto& request_ref = static_cast<BaseBusRequest&>(*request_ptr);
				handler.AddBus(request_ref);
			}
		}
		for (auto& request_ptr : requests)	// adding distances
		{
			if (request_ptr->type == RequestType::STOP)
			{
				auto& request_ref = static_cast<BaseStopRequest&>(*request_ptr);
				for (auto& [stop_name, distance] : request_ref.road_distances)
				{
					handler.AddDistance(request_ref.name, stop_name, distance);
				}
			}
		}
	}

	//---------------------------------------------------------------------------------------


	StatRequest::StatRequest(int no, RequestType t, std::string n)
		: id(no),
		type(t),
		name(n) {}

	StatRequestHandler::StatRequestHandler(const TransportCatalogue& catalogue)
		: catalogue_(catalogue) {}

	BusInfo StatRequestHandler::GetBusInfo(StatRequest& request) const
	{
		BusInfo result = catalogue_.GetBusInfo(request.name);
		result.id = request.id;
		return result;
	}

	StopInfo StatRequestHandler::GetStopInfo(StatRequest& request) const
	{
		StopInfo result = catalogue_.GetStopInfo(request.name);
		result.id = request.id;
		return result;
	}

	std::vector<std::unique_ptr<Info>> ProcessStatRequests
	(std::vector<std::unique_ptr<StatRequest>> requests, StatRequestHandler handler)
	{
		std::vector < std::unique_ptr<Info>> result;

		for (auto& request_ptr : requests)
		{
			if (request_ptr->type == RequestType::STOP)
			{
				auto answer_ptr = std::make_unique<StopInfo>(handler.GetStopInfo(*request_ptr));
				result.push_back(std::move(answer_ptr));
			}
			else if (request_ptr->type == RequestType::BUS)
			{
				auto answer_ptr = std::make_unique<BusInfo>(handler.GetBusInfo(*request_ptr));
				result.push_back(std::move(answer_ptr));
			}
		}

		return result;

	}
}
