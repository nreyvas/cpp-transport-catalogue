#pragma once
/*
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "transport_catalogue.h"

namespace input_reader
{
	using namespace transport_catalogue;

	enum class InputRequestType
	{
		NEW_STOP,
		NEW_DISTANCES,
		NEW_ROUTE
	};

	struct Inquiry
	{
		InputRequestType type;
		std::string name;
		std::string text;
	};

	void ProcessInquiresFromStream(std::istream& is, TransportCatalogue& catalogue);

	void ProcessInquires(std::vector<Inquiry> inquiries, TransportCatalogue& catalogue);

	void AddStop(Inquiry& inq, TransportCatalogue& catalogue);

	void AddDistances(Inquiry& inq, TransportCatalogue& catalogue);

	void AddRoute(Inquiry& inq, TransportCatalogue& catalogue);

	std::vector<std::string> SplitIntoWords(std::string text);

	std::vector<std::string> SplitIntoStopNames(std::string text);

	void ClearFromSpaces(std::string& text);

	// returns inquiry type and erases the type word from text
	InputRequestType ExtractInquiryType(std::string& text);

	// should be used after ExtractInquiryType, return name of bus/route and erases it from text
	std::string ExtractInquiryName(std::string& text);

	// Splits text into a string with coordinates and a string with distances
	// ExtractInquiryType and ExtractInquiryName should be used before
	void SplitCoordinatesAndDistances(std::string& text,
		std::string& stop_inquiry_text, std::string& distances_inquiry_text);
}
*/