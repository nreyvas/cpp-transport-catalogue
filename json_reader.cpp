#include <algorithm>

#include "json_reader.h"

namespace transport_catalogue
{
	JsonReader::JsonReader(std::istream& input, std::ostream& output)
		: input_(input),
		output_(output) {}

	void JsonReader::ProcessRequests()
	{
		json_requests_ = json::LoadNode(input_);
	}

	std::vector<std::unique_ptr<BaseRequest>> JsonReader::GetBaseRequests()
	{
		const std::map<std::string, json::Node>& requests_map = json_requests_.AsMap();
		const std::vector<json::Node>& base_requests_array = requests_map.at("base_requests"s).AsArray();

		std::vector<std::unique_ptr<BaseRequest>> result;
		result.reserve(base_requests_array.size());

		for (const json::Node& node_request : base_requests_array)
		{
			const json::Dict& request = node_request.AsMap();
			if (request.at("type"s).AsString() == "Bus")
			{
				std::vector<std::string> stops;
				stops.reserve(request.at("stops").AsArray().size());
				for (const auto& stop_node : request.at("stops").AsArray())
				{
					stops.push_back(stop_node.AsString());
				}

				auto new_request_ptr = std::make_unique<BaseBusRequest>(
					RequestType::BUS,
					request.at("name"s).AsString(),
					stops,
					request.at("is_roundtrip").AsBool());

				result.push_back(std::move(new_request_ptr));
			}
			else if (request.at("type"s).AsString() == "Stop")
			{
				std::map<std::string, int> road_distances;
				for (const auto& [stop_name, distance_node] : request.at("road_distances").AsMap())
				{
					road_distances.insert({ stop_name, distance_node.AsInt() });
				}
				auto new_request_ptr = std::make_unique<BaseStopRequest>(
					RequestType::STOP,
					request.at("name"s).AsString(),
					geo::Coordinates
					{
						request.at("latitude").AsDouble(),
						request.at("longitude").AsDouble()
					},
					road_distances);

				result.push_back(std::move(new_request_ptr));
			}
		}
		return result;
	}

	renderer::MapRenderer JsonReader::GetRenderSettings() const
	{
		const std::map<std::string, json::Node>& requests_map = json_requests_.AsMap();
		const json::Dict settings_map = requests_map.at("render_settings"s).AsMap();

		renderer::MapRenderer renderer;

		renderer.SetWidth(settings_map.at("width"s).AsDouble());
		renderer.SetHeight(settings_map.at("height"s).AsDouble());
		renderer.SetPadding(settings_map.at("padding"s).AsDouble());
		renderer.SetLineWidth(settings_map.at("line_width"s).AsDouble());
		renderer.SetStopRadius(settings_map.at("stop_radius"s).AsDouble());
		renderer.SetBusLabelFontSize(settings_map.at("bus_label_font_size"s).AsInt());

		json::Array bus_label_offset_node = settings_map.at("bus_label_offset"s).AsArray();
		svg::Point bus_label_offset{
			bus_label_offset_node[0].AsDouble(),
			bus_label_offset_node[1].AsDouble() };
		renderer.SetBusLabelOffset(bus_label_offset);
		renderer.SetStopLabelFontSize(settings_map.at("stop_label_font_size"s).AsInt());

		json::Array stop_label_offset_node = settings_map.at("stop_label_offset"s).AsArray();
		svg::Point stop_label_offset{
			stop_label_offset_node[0].AsDouble(),
			stop_label_offset_node[1].AsDouble() };
		renderer.SetStopLabelOffset(stop_label_offset);

		svg::Color underlayer_color = ProcessColorFromNode(settings_map.at("underlayer_color"s));
		renderer.SetUnderlayerColor(underlayer_color);
		renderer.SetUnderlayerWidth(settings_map.at("underlayer_width"s).AsDouble());

		json::Array color_pallete_node_array = settings_map.at("color_palette"s).AsArray();
		std::vector<svg::Color> color_palette;
		for (const json::Node color_node : color_pallete_node_array)
		{
			color_palette.push_back(ProcessColorFromNode(color_node));
		}

		renderer.SetColorPalette(color_palette);

		return renderer;
	}


	std::vector<std::unique_ptr<StatRequest>> JsonReader::GetStatRequests()
	{
		const std::map<std::string, json::Node>& requests_map = json_requests_.AsMap();
		const std::vector<json::Node>& stat_requests_array = requests_map.at("stat_requests"s).AsArray();

		std::vector<std::unique_ptr<StatRequest>> result;
		result.reserve(stat_requests_array.size());

		for (const json::Node& node_request : stat_requests_array)
		{
			RequestType type;
			if (node_request.AsMap().at("type"s).AsString() == "Bus")
			{
				type = RequestType::BUS;
			}
			else if (node_request.AsMap().at("type"s).AsString() == "Stop")
			{
				type = RequestType::STOP;
			}
			else
			{
				type = RequestType::MAP;
			}

			std::string name{};
			if (type != RequestType::MAP)
			{
				name = node_request.AsMap().at("name"s).AsString();
			}

			auto new_request_ptr = std::make_unique<StatRequest>(
				node_request.AsMap().at("id"s).AsInt(), type, name);

			result.push_back(std::move(new_request_ptr));
		}
		return result;
	}

	void JsonReader::FillAnswers(std::vector<std::unique_ptr<Info>> answers)
	{
		for (const auto& answer_ptr : answers)
		{
			if (!(answer_ptr->IsFound))
			{
				json::Dict answer_map
				{
					{"request_id", json::Node{answer_ptr->id}},
					{"error_message", json::Node{"not found"s}}
				};
				json_answers_.PushBack(json::Node{ answer_map });
			}
			else if (answer_ptr->type == RequestType::BUS)
			{
				auto& answer_ref = static_cast<BusInfo&>(*answer_ptr);

				json::Dict answer_map
				{ {"curvature"s, json::Node{answer_ref.curvature}},
					{"request_id"s, json::Node{answer_ref.id}},
					{"route_length"s, json::Node{answer_ref.length}},
					{"stop_count"s, json::Node{answer_ref.stop_count}},
					{"unique_stop_count"s, json::Node{answer_ref.unique_stop_count}}
				};
				
				json_answers_.PushBack(json::Node{ answer_map });
			}
			else if (answer_ptr->type == RequestType::STOP)
			{
				auto& answer_ref = static_cast<StopInfo&>(*answer_ptr);

				std::vector<json::Node> bus_node_list;
				bus_node_list.reserve(answer_ref.routes.size());
				for (const Bus* bus_ptr : answer_ref.routes)
				{
					bus_node_list.push_back(json::Node{ bus_ptr->name });
				}
				std::sort(bus_node_list.begin(), bus_node_list.end(), [](const auto& lhs, const auto& rhs)
					{ return lhs.AsString() < rhs.AsString(); });
				json::Dict answer_map
				{
					{"buses", bus_node_list},
					{"request_id", json::Node{answer_ref.id}}
				};

				json_answers_.PushBack(json::Node{ answer_map });
			}
			else if (answer_ptr->type == RequestType::MAP)
			{
				auto& answer_ref = static_cast<MapInfo&>(*answer_ptr);
				json::Dict answer_map
				{
					{"map", json::Node{answer_ref.svg_code}},
					{"request_id", json::Node{answer_ref.id}}
				};

				json_answers_.PushBack(json::Node{ answer_map });
			}
		}
	}

	void JsonReader::OutputAnswers()
	{
		json::PrintNode(json_answers_, output_);
	}

	svg::Color JsonReader::ProcessColorFromNode(const json::Node& color_node) const
	{
		svg::Color result;
		if (color_node.IsString())
		{
			result = color_node.AsString();
		}
		else if (color_node.IsArray())
		{
			json::Array color_array = color_node.AsArray();
			if (color_array.size() == 3)
			{
				svg::Rgb color;
				color.red = color_array[0].AsInt();
				color.green = color_array[1].AsInt();
				color.blue = color_array[2].AsInt();
				result = color;
			}
			else if (color_array.size() == 4)
			{
				svg::Rgba color;
				color.red = color_array[0].AsInt();
				color.green = color_array[1].AsInt();
				color.blue = color_array[2].AsInt();
				color.opacity = color_array[3].AsDouble();
				result = color;
			}
		}
		return result;
	}
}