#include <iostream>
#include <fstream>
#include <sstream>


#include "request_handler.h"
#include "json_reader.h"

using namespace transport_catalogue;

int main()
{
	
	TransportCatalogue catalogue;
	
	
	std::string input_json = "D:/CPP/YANDEX/cpp-transport-catalogue/tests/RouteTest03.json";
	std::ifstream ifs{ input_json };
	
	//JsonReader json_reader(ifs, std::cout);

	JsonReader json_reader(std::cin, std::cout);
	json_reader.ProcessRequests();

	renderer::MapRenderer renderer = json_reader.GetRenderSettings();
	RequestHandler request_handler{ catalogue, renderer };
	request_handler.SetRouterSettings(json_reader.GetRoutingSettings().first,
									json_reader.GetRoutingSettings().second);
	ProcessBaseRequests(json_reader.GetBaseRequests(), request_handler);

	json_reader.FillAnswers(
		ProcessStatRequests(json_reader.GetStatRequests(), request_handler));

	json_reader.OutputAnswers();
}