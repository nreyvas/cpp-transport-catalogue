#include <algorithm>
#include <sstream>

#include "request_handler.h"


namespace transport_catalogue
{
	BaseRequest::BaseRequest(RequestType t, std::string n)
		: type(t),
		name(n) {}


	BaseStopRequest::BaseStopRequest
	(RequestType t, std::string n, geo::Coordinates c, std::map<std::string, int> rd)
		: BaseRequest(t, n),
		coords(c),
		road_distances(rd) {}

	BaseBusRequest::BaseBusRequest
	(RequestType t, std::string n, std::vector<std::string> s, bool ir)
		: BaseRequest(t, n),
		stops(s),
		is_roundtrip(ir) {}

	StatRequest::StatRequest(int no, RequestType t, std::string n)
		: id(no),
		type(t),
		name(n) {}

	//---------------------------------------------------------------------------------------

	RequestHandler::RequestHandler(TransportCatalogue& catalogue, const renderer::MapRenderer& renderer)
		: catalogue_(catalogue),
		renderer_(std::move(renderer)) {}

	void RequestHandler::AddStop(BaseStopRequest& inquiry)
	{
		catalogue_.AddStop(inquiry.name, inquiry.coords);
	}

	void RequestHandler::AddDistance(std::string stop_from, std::string stop_to, int distance)
	{
		catalogue_.AddDistance(stop_from, stop_to, distance);
	}

	void RequestHandler::AddBus(BaseBusRequest& inquiry)
	{
		catalogue_.AddRoute(std::move(inquiry.name), std::move(inquiry.stops), inquiry.is_roundtrip);
	}

	BusInfo RequestHandler::GetBusInfo(StatRequest& request) const
	{
		BusInfo result = catalogue_.GetBusInfo(request.name);
		result.id = request.id;
		return result;
	}

	StopInfo RequestHandler::GetStopInfo(StatRequest& request) const
	{
		StopInfo result = catalogue_.GetStopInfo(request.name);
		result.id = request.id;
		return result;
	}

	void ProcessBaseRequests(std::vector<std::unique_ptr<BaseRequest>> requests,
		RequestHandler handler)
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

	
	svg::Document RequestHandler::RenderMap() const
	{
		svg::Document result;

		std::vector<const Bus*> buses = catalogue_.GetBuses();

		// create a SphereProjector for converting coordinates into points
		std::vector<geo::Coordinates> stop_coords;
		for (const Bus* bus_ptr : buses)
		{
			for (const Stop* stop_ptr : bus_ptr->stops)
			{
				stop_coords.push_back(stop_ptr->coordinates);
			}
		}
		const renderer::SphereProjector proj{ stop_coords.begin(), stop_coords.end(),
			renderer_.GetWidth(), renderer_.GetHeight(), renderer_.GetPadding() };

		// generate polylines representing buses
		std::vector<svg::Polyline> poly_buses = GenerateBuses(proj);
		for (svg::Polyline& bus_poly : poly_buses)
		{
			result.Add(bus_poly);
		}

		// generate bus names
		std::vector<svg::Text> text_buses = GenerateBusNames(proj);
		for (svg::Text& bus_name : text_buses)
		{
			result.Add(bus_name);
		}
		
		// generate stops
		std::vector<svg::Circle> circle_stops = GenerateStops(proj);
		for (svg::Circle& circle_stops : circle_stops)
		{
			result.Add(circle_stops);
		}
		
		// generate stop names
		std::vector<svg::Text> text_stops = GenerateStopNames(proj);
		for (svg::Text& stop_name : text_stops)
		{
			result.Add(stop_name);
		}
		
		return result;
	}
	
	std::vector<svg::Polyline> RequestHandler::GenerateBuses(const renderer::SphereProjector& proj) const
	{
		std::vector<const Bus*> buses = catalogue_.GetBuses();
		std::vector<svg::Polyline> poly_buses;
		size_t color_id = 0;
		for (const Bus* bus_ptr : buses)
		{
			if (bus_ptr->stops.empty())	// if the bus has no stops - skip it
			{
				continue;
			}
			svg::Polyline poly_bus;
			for (const Stop* stop_ptr : bus_ptr->stops)
			{
				poly_bus.AddPoint(proj(stop_ptr->coordinates));
			}
			if (!(bus_ptr->is_circular))
			{
				for (int i = int(bus_ptr->stops.size()) - 2; i >= 0; --i)
				{
					poly_bus.AddPoint(proj(bus_ptr->stops[i]->coordinates));
				}
			}
			poly_bus.SetStrokeWidth(renderer_.GetLineWidth());
			poly_bus.SetStrokeColor(renderer_.GetColorPalette()[color_id]);
			poly_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			poly_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			poly_buses.push_back(poly_bus);
			++color_id;
			if (color_id == renderer_.GetColorPalette().size())
			{
				color_id = 0;
			}
		}
		return poly_buses;
	}

	std::vector<svg::Text> RequestHandler::GenerateBusNames(const renderer::SphereProjector& proj) const
	{
		std::vector<const Bus*> buses = catalogue_.GetBuses();
		std::vector<svg::Text> text_buses;
		size_t color_id = 0;
		for (const Bus* bus_ptr : buses)
		{
			if (bus_ptr->stops.empty())	// if the bus has no stops - skip it
			{
				continue;
			}
			svg::Text first_bus_name;
			first_bus_name.SetPosition(proj(bus_ptr->stops[0]->coordinates));
			first_bus_name.SetOffset(renderer_.GetBusLabelOffset());
			first_bus_name.SetFontSize(renderer_.GetBusLabelFontSize());
			first_bus_name.SetFontFamily("Verdana"s);
			first_bus_name.SetFontWeight("bold"s);
			first_bus_name.SetData(bus_ptr->name);
			first_bus_name.SetFillColor(renderer_.GetColorPalette()[color_id]);

			svg::Text first_bus_name_back = first_bus_name;
			first_bus_name_back.SetFillColor(renderer_.GetUnderlayerColor());
			first_bus_name_back.SetStrokeColor(renderer_.GetUnderlayerColor());
			first_bus_name_back.SetStrokeWidth(renderer_.GetUnderlayerWidth());
			first_bus_name_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			first_bus_name_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			text_buses.push_back(first_bus_name_back);
			text_buses.push_back(first_bus_name);

			if (!(bus_ptr->is_circular) && (bus_ptr->stops[0] != bus_ptr->stops[bus_ptr->stops.size() - 1]))
			{
				svg::Text second_bus_name = first_bus_name;
				svg::Text second_bus_name_back = first_bus_name_back;
				second_bus_name.SetPosition(proj(bus_ptr->stops[bus_ptr->stops.size() - 1]->coordinates));
				second_bus_name_back.SetPosition(proj(bus_ptr->stops[bus_ptr->stops.size() - 1]->coordinates));
				text_buses.push_back(second_bus_name_back);
				text_buses.push_back(second_bus_name);
			}

			++color_id;
			if (color_id == renderer_.GetColorPalette().size())
			{
				color_id = 0;
			}
		}
		return text_buses;
	}

	std::vector<svg::Circle> RequestHandler::GenerateStops(const renderer::SphereProjector& proj) const
	{
		std::vector<const Stop*> stops = catalogue_.GetActiveStops();
		std::vector<svg::Circle> stop_circles;
		for (const Stop* stop_ptr : stops)
		{
			svg::Circle stop_circle;
			stop_circle.SetCenter(proj(stop_ptr->coordinates));
			stop_circle.SetRadius(renderer_.GetStopRadius());
			stop_circle.SetFillColor("white");
			stop_circles.push_back(stop_circle);
		}
		return stop_circles;
	}

	std::vector<svg::Text> RequestHandler::GenerateStopNames(const renderer::SphereProjector& proj) const
	{
		std::vector<const Stop*> stops = catalogue_.GetActiveStops();
		std::vector<svg::Text> text_stops;
		for (const Stop* stop_ptr : stops)
		{
			svg::Text stop_name;
			stop_name.SetPosition(proj(stop_ptr->coordinates));
			stop_name.SetOffset(renderer_.GetStopLabelOffset());
			stop_name.SetFontSize(renderer_.GetStopLabelFontSize());
			stop_name.SetFontFamily("Verdana"s);
			stop_name.SetData(stop_ptr->name);
			stop_name.SetFillColor("black"s);

			svg::Text stop_name_back = stop_name;
			stop_name_back.SetFillColor(renderer_.GetUnderlayerColor());
			stop_name_back.SetStrokeColor(renderer_.GetUnderlayerColor());
			stop_name_back.SetStrokeWidth(renderer_.GetUnderlayerWidth());
			stop_name_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			stop_name_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			text_stops.push_back(stop_name_back);
			text_stops.push_back(stop_name);
		}
		return text_stops;
	}

	std::vector<std::unique_ptr<Info>> ProcessStatRequests
	(std::vector<std::unique_ptr<StatRequest>> requests, RequestHandler handler)
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
			else
			{
				std::unique_ptr<MapInfo> map_ptr{ new MapInfo{} };
				map_ptr->id = request_ptr->id;
				map_ptr->type = RequestType::MAP;
				map_ptr->is_found = true;

				std::ostringstream oss;
				handler.RenderMap().Render(oss);
				map_ptr->svg_code = oss.str();

				result.push_back(std::move(map_ptr));
			}
		}
		return result;
	}
}
