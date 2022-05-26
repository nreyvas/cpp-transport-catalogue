#include "domain.h"

namespace transport_catalogue
{
	Info::Info(int no, std::string n, RequestType t, bool found)
		: id(no),
		name(std::move(n)),
		type(t),
		IsFound(found) {}

	BusInfo::BusInfo(int no, std::string n, bool found)
		: Info(no, n, RequestType::BUS, found) {}

	StopInfo::StopInfo(int no, std::string n, bool found)
		: Info(no, n, RequestType::STOP, found) {}

	size_t StopPairHash::operator() (const std::pair<const Stop*, const Stop*> p) const
	{
		return hasher_(p.first->name) * 37 + hasher_(p.second->name);
	}
}