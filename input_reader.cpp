#include "sstream"

#include "input_reader.h"
/*
namespace input_reader
{
	void ProcessInquiresFromStream(std::istream& is, TransportCatalogue& catalogue)
	{
		int inquire_amount;
		is >> inquire_amount;
		is.ignore(32767, '\n');
		std::vector<Inquiry> inquiries;
		inquiries.reserve(inquire_amount);

		for (int i = 0; i < inquire_amount; ++i)
		{
			std::string line;
			std::getline(is, line);
			Inquiry inq;
			inq.type = ExtractInquiryType(line);
			inq.name = ExtractInquiryName(line);
			if (inq.type == InputRequestType::NEW_DISTANCES)
			{
				inq.type = InputRequestType::NEW_STOP;
				Inquiry dist_inq;
				dist_inq.name = inq.name;
				dist_inq.type = InputRequestType::NEW_DISTANCES;
				SplitCoordinatesAndDistances(line, inq.text, dist_inq.text);
				inquiries.push_back(dist_inq);
			}
			else
			{
				inq.text = std::move(line);
			}
			inquiries.push_back(inq);
		}

		std::sort(inquiries.begin(), inquiries.end(), [](const Inquiry& lhs, const Inquiry& rhs)
			{ return lhs.type < rhs.type; });

		ProcessInquires(inquiries, catalogue);
	}

	void ProcessInquires(std::vector<Inquiry> inquiries, TransportCatalogue& catalogue)
	{
		for (Inquiry& inquiry : inquiries)
		{
			if (inquiry.type == InputRequestType::NEW_STOP)
			{
				AddStop(inquiry, catalogue);
			}
			else if (inquiry.type == InputRequestType::NEW_DISTANCES)
			{
				AddDistances(inquiry, catalogue);
			}
			else
			{
				AddRoute(inquiry, catalogue);
			}
		}
	}

	void AddStop(Inquiry& inq, TransportCatalogue& catalogue)
	{
		size_t comma_pos = inq.text.find(',');
		inq.text.erase(comma_pos, 1);
		std::istringstream iss{ std::move(inq.text) };
		Coordinates coordinates{};
		iss >> coordinates.lat;
		iss >> coordinates.lng;
		catalogue.AddStop(std::move(inq.name), coordinates);
	}

	void AddDistances(Inquiry& inq, TransportCatalogue& catalogue)
	{
		while (true)
		{
			size_t m_pos = inq.text.find('m');
			std::string distance_string = inq.text.substr(0, m_pos);
			std::istringstream iss{ std::move(distance_string) };
			int distance;
			iss >> distance;
			inq.text.erase(0, m_pos + 5); // removes "XXXXm to " from the start
			size_t comma_pos = inq.text.find(',');
			std::string stop_to_name = inq.text.substr(0, comma_pos);
			catalogue.AddDistance(inq.name, std::move(stop_to_name), distance);
			if (comma_pos == inq.text.npos)
			{
				break;
			}
			else
			{
				inq.text.erase(0, comma_pos + 2); // removes name + ", " from the start
			}
		}
	}

	void AddRoute(Inquiry& inq, TransportCatalogue& catalogue)
	{
		bool IsCircular = true;	// checking if the route is circular
		if (inq.text.find('>') == inq.text.npos)
		{
			IsCircular = false;
		}

		std::vector<std::string> stop_names = SplitIntoStopNames(std::move(inq.text));

		catalogue.AddRoute(std::move(inq.name), stop_names, IsCircular);
	}

	std::vector<std::string> SplitIntoWords(std::string text)
	{
		std::vector<std::string> words;
		std::istringstream iss(text);
		std::string word;
		while (true)
		{
			std::string word;
			iss >> word;
			if (!iss) break;
			words.push_back(std::move(word));
			word.clear();
		}
		return words;
	}

	std::vector<std::string> SplitIntoStopNames(std::string text)
	{
		std::vector<std::string> stop_names;
		while (!text.empty())
		{
			size_t separator_pos = text.find_first_of(">-");
			std::string name = text.substr(0, separator_pos);
			ClearFromSpaces(name);
			stop_names.push_back(std::move(name));
			if (separator_pos == text.npos)
			{
				break;
			}
			text.erase(0, separator_pos + 1);
		}
		return stop_names;
	}

	void ClearFromSpaces(std::string& text)
	{
		size_t word_start = text.find_first_not_of(' ');
		text.erase(0, word_start);
		size_t word_end = text.find_last_not_of(' ');
		if (word_end != text.npos)
		{
			text.erase(word_end + 1);
		}
	}

	// returns inquiry type and erases the type word from text
	InputRequestType ExtractInquiryType(std::string& text)
	{
		size_t space_position = text.find(' ');
		std::string inq_type = text.substr(0, space_position);
		text.erase(0, space_position + 1);
		if (inq_type == "Bus")
		{
			return InputRequestType::NEW_ROUTE;
		}
		if (text.find("m to ") != text.npos)
		{
			return InputRequestType::NEW_DISTANCES;
		}
		return InputRequestType::NEW_STOP;
	}


	// should be used after ExtractInquiryType, return name of bus/route and erases it from text
	std::string ExtractInquiryName(std::string& text)
	{
		size_t colon_pos = text.find(':');
		std::string name = text.substr(0, colon_pos);
		text.erase(0, colon_pos + 2);
		return name;
	}

	// Splits text into a string with coordinates and a string with distances
	// ExtractInquiryType and ExtractInquiryName should be used before
	void SplitCoordinatesAndDistances(std::string& text,
		std::string& stop_inquiry_text, std::string& distances_inquiry_text)
	{
		size_t first_comma_pos = text.find(',');
		size_t second_comma_pos = text.find(',', first_comma_pos + 1);
		stop_inquiry_text = text.substr(0, second_comma_pos);
		text.erase(0, second_comma_pos + 2);
		distances_inquiry_text = std::move(text);
	}
}
*/
