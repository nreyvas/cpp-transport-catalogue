#include <iostream>
#include <fstream>
#include <sstream>


#include "request_handler.h"
#include "json_reader.h"

//#include "transport_catalogue.h"
//#include "input_reader.h"
//#include "stat_reader.h"

/*
int main()	// previous main
{
	transport_catalogue::TransportCatalogue catalogue;

	input_reader::ProcessInquiresFromStream(std::cin, catalogue);
	stat_reader::ProcessInquiresFromStream(std::cin, std::cout, catalogue);
}
*/

/*
int main()	// previous tests
{
	using namespace transport_catalogue;
	TransportCatalogue catalogue;

	std::string test_input = "D:/CPP/YANDEX/cpp-transport-catalogue/tests/test03.txt"s;
	std::string test_output = "D:/CPP/YANDEX/cpp-transport-catalogue/tests/tsA_case1_output.txt"s;

	std::ifstream ifs(test_input);
	input_reader::ProcessInquiresFromStream(ifs, catalogue);
	
	std::ostringstream oss;
	stat_reader::ProcessInquiresFromStream(ifs, std::cout, catalogue);
	
	std::string result = oss.str();

	std::ifstream ifs_result(test_output);
	std::stringstream result_read_from_file;
	result_read_from_file << ifs_result.rdbuf();
	std::string result_from_file(result_read_from_file.str());

	std::string my_output = "D:/CPP/YANDEX/cpp-transport-catalogue/tests/tsA_case1_my_output.txt"s;
	std::ofstream my_output_ofs(my_output);
	my_output_ofs << result;

	std::cout << "\ndone";
	
}
*/

using namespace transport_catalogue;

int main()
{
	
	TransportCatalogue catalogue;
	
	std::string input_json = "D:/CPP/YANDEX/cpp-transport-catalogue/input2.json";
	std::ifstream ifs{ input_json };
	
	JsonReader json_reader(ifs, std::cout);

	//JsonReader json_reader(std::cin, std::cout);
	json_reader.ProcessRequests();

	ProcessBaseRequests(json_reader.GetBaseRequests(), BaseRequestHandler(catalogue));
	
	json_reader.FillAnswers(
		ProcessStatRequests(json_reader.GetStatRequests(), StatRequestHandler(catalogue)));

	json_reader.OutputAnswers();
	

	//std::cout << "done\n";
}





