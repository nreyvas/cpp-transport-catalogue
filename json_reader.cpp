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
					Coordinates
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

	std::vector<std::unique_ptr<StatRequest>> JsonReader::GetStatRequests()
	{
		const std::map<std::string, json::Node>& requests_map = json_requests_.AsMap();
		const std::vector<json::Node>& stat_requests_array = requests_map.at("stat_requests"s).AsArray();

		std::vector<std::unique_ptr<StatRequest>> result;
		result.reserve(stat_requests_array.size());

		for (const json::Node& node_request : stat_requests_array)
		{
			RequestType type = (node_request.AsMap().at("type"s).AsString() == "Bus") ?
				RequestType::BUS : RequestType::STOP;

			auto new_request_ptr = std::make_unique<StatRequest>(
				node_request.AsMap().at("id"s).AsInt(),
				type,
				node_request.AsMap().at("name"s).AsString());

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
				for (const Route* bus_ptr : answer_ref.routes)
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
		}
	}

	void JsonReader::OutputAnswers()
	{
		json::PrintNode(json_answers_, output_);
	}


}