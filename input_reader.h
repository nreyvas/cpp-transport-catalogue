#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "transport_catalogue.h"

namespace input_reader
{
	enum class INQUIRY_TYPE
	{
		NEW_STOP,
		NEW_ROUTE
	};

	struct Inquiry
	{
		INQUIRY_TYPE type;
		std::string text;

	};

	void ProcessInquiresFromStream(std::istream& is, TransportCatalogue& catalogue);

	Inquiry ProcessLine(std::string line);

	void ProcessInquires(std::vector<Inquiry> inquiries, TransportCatalogue& catalogue);

	void AddStop(std::string text, TransportCatalogue& catalogue);

	void AddRoute(std::string text, TransportCatalogue& catalogue);

	std::vector<std::string> SplitIntoWords(std::string text);

	std::vector<std::string> SplitIntoStopNames(std::string text);

	void ClearFromSpaces(std::string& text);
}