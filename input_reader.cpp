#include "sstream"

#include "input_reader.h"

namespace input_reader
{
	void ProcessInquiresFromStream(std::istream& is, TransportCatalogue& catalogue)
	{
		int inquire_amount;
		is >> inquire_amount;
		is.ignore(32767, '\n');
		std::vector<Inquiry> inquiries(inquire_amount);
		for (int i = 0; i < inquire_amount; ++i)
		{
			std::string line;
			std::getline(is, line);
			inquiries[i] = ProcessLine(line);
		}
		std::sort(inquiries.begin(), inquiries.end(), [](const Inquiry& lhs, const Inquiry& rhs)
			{ return lhs.type < rhs.type; });

		ProcessInquires(inquiries, catalogue);
	}

	Inquiry ProcessLine(std::string line)
	{
		Inquiry inquiry;

		inquiry.type = (line[0] == 'S') ? INQUIRY_TYPE::NEW_STOP : INQUIRY_TYPE::NEW_ROUTE;
		int inquiry_start = line.find(' ') + 1;
		inquiry.text = std::move(line.erase(0, inquiry_start));

		return inquiry;
	}

	void ProcessInquires(std::vector<Inquiry> inquiries, TransportCatalogue& catalogue)
	{
		for (Inquiry& inquiry : inquiries)
		{
			if (inquiry.type == INQUIRY_TYPE::NEW_STOP)
			{
				AddStop(std::move(inquiry.text), catalogue);
			}
			else
			{
				//std::cerr << "adding route: "s << inquiry.text << '\n';
				AddRoute(std::move(inquiry.text), catalogue);
			}
		}
	}

	void AddStop(std::string text, TransportCatalogue& catalogue)
	{
		size_t colon_pos = text.find(':');
		std::string name = text.substr(0, colon_pos);
		text.erase(0, colon_pos + 2);
		std::vector<std::string> words = SplitIntoWords(std::move(text));

		words[words.size() - 2].pop_back(); // remove comma
		std::istringstream iss{ std::move(words[words.size() - 2])};
		Coordinates coordinates;
		iss >> coordinates.lat;
		iss.clear();
		iss.str(std::move(words[words.size() - 1]));
		iss >> coordinates.lng;

		catalogue.AddStop(std::move(name), coordinates);
	}

	void AddRoute(std::string text, TransportCatalogue& catalogue)
	{
		size_t colon_pos = text.find(':');
		std::string name = text.substr(0, colon_pos);
		text.erase(0, colon_pos + 2);

		bool IsCircular = true;	// checking if the route is circular
		if (text.find('>') == text.npos)
		{
			IsCircular = false;
		}

		std::vector<std::string> stop_names = SplitIntoStopNames(std::move(text));

		catalogue.AddRoute(name, stop_names, IsCircular);
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
}

