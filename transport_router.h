#pragma once
#include <unordered_map>

#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

namespace router
{
	const int MAX_STOP_AMOUNT = 100;

	using namespace transport_catalogue;

	class TransportRouter
	{
	private:

		TransportCatalogue& catalogue_;
		graph::DirectedWeightedGraph<Span> graph_;
		graph::Router<Span>* router_ = nullptr;

		std::unordered_map<int, const Stop*> id_to_stop_;
		std::unordered_map<const Stop*, int> stop_to_id_;
		int bus_wait_time_ = 1;
		double bus_velocity_ = 1;
		size_t stop_counter_ = 0;


	public:

		TransportRouter(TransportCatalogue& catalogue)
			: catalogue_(catalogue),
			graph_(MAX_STOP_AMOUNT) {}

		~TransportRouter()
		{
			delete router_;
		}

		void SetBusWaitTime(int time);
		void SetBusVelocity(double vel);

		int GetBusWaitTime() const;
		double GetBusVelocity() const;

		void AddStop(const transport_catalogue::Stop& stop);
		void AddBus(const transport_catalogue::Bus& bus);

		RouteInfo GetRouteInfo(const Stop* from_stop, const Stop* to_stop) const;

		void Activate();

	};
}