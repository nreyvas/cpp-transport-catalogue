#include <iostream>
#include <fstream>
#include <sstream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main()
{
	transport_catalogue::TransportCatalogue catalogue;

	input_reader::ProcessInquiresFromStream(std::cin, catalogue);
	stat_reader::ProcessInquiresFromStream(std::cin, std::cout, catalogue);
}





