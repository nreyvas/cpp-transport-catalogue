#pragma once

#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

namespace transport_catalogue
{
	class JsonReader
	{
	public:		

		JsonReader(std::istream& input, std::ostream& output);

		void ProcessRequests();	// reads and sorts requests

		std::vector<std::unique_ptr<BaseRequest>> GetBaseRequests();

		renderer::MapRenderer GetRenderSettings() const;

		std::pair<int, double> GetRoutingSettings() const;	// gets bus waiting time and bus velocity

		std::vector<std::unique_ptr<StatRequest>> GetStatRequests();

		void FillAnswers(std::vector<std::unique_ptr<Info>> answers);

		void OutputAnswers();

	private:

		std::istream& input_;
		std::ostream& output_;
		json::Node json_requests_;
		json::Node json_answers_{ json::Array{} };

		svg::Color ProcessColorFromNode(const json::Node& color_node) const;
	};
}