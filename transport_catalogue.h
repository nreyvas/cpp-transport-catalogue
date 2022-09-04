#pragma once

#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <utility>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <functional>
#include <set>
#include <map>
#include <cmath>
#include "domain.h"
#include "router.h"



namespace catalogue
{

using geo::Coordinates;
using geo::ComputeDistance;




struct BusStat {
    unsigned int stops_count = 0;
    unsigned int unique_stops_count = 0;
    uint64_t length = 0;
    double curvative = 0;

    bool IsEmpty() const {
        return stops_count == 0 && unique_stops_count == 0 && length < 1e-6 && curvative == 0;
    }
    bool IsExsists = false;
};

struct StopInfo {
    std::set<std::string_view> buses_;
    bool IsExsists = false; 
};

struct RouteInfo {

};



struct RouteDistanceHasher {
    size_t operator()(std::pair<const Stop*, const Stop*> p) const {
        std::hash<const void*> ptr_hasher{}; // объект - хешер
        static const size_t PRIME = 13;
        return PRIME * ptr_hasher(p.first) + ptr_hasher(p.second);
    }
};

class TransportCatalogue {
public:
    
    TransportCatalogue() = default;
    // TransportCatalogue(RoutingSettings routing_settings);

    void AddBus(std::string_view name, const std::vector<std::string>& stops, BusType type);

    void AddStop(std::string_view name, Coordinates coordinates);

    const Stop* FindStop(std::string_view name) const;

    const Bus* FindBus(std::string_view name) const;

    BusStat GetBusInfo(std::string_view name) const;

    StopInfo GetStopInfo(std::string_view stop_name) const;

    void PrintIntervals() const;

    void SetDistance(std::pair<const Stop*, const Stop*> p, uint64_t distance);

    std::deque<const Bus*> GetBusesSorted() const;
    const std::unordered_map<const Stop*, std::set<const Bus*>>& GetStopsToBuses() const;
    const std::map<std::string_view, const Stop*>& GetStopnameToStops() const;

    // ------- for serialization purposes
    const std::deque<Stop>& GetStops() const;
    const std::deque<Bus>& GetBuses() const;
    const std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, RouteDistanceHasher>& GetIntervalsToDistance() const;
    
    void SetStops(std::deque<Stop>&& stops);
    void SetStopnameToStop(std::map<std::string_view, const Stop*>&& stopname_to_stop);
    void SetBuses(std::deque<Bus>&& buses);
    void SetBusnameToBus(std::map<std::string_view, const Bus*>&& busname_to_bus);
    void SetStopsToBuses(std::unordered_map<const Stop*, std::set<const Bus*>>&& stops_to_buses);
    void SetIntervalsToDistance(std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, catalogue::RouteDistanceHasher>&& intervals_to_distance);

private:
    
    //все остановки в базе данных
    std::deque<Stop> stops_; 
    //индексация остановок -> указатель на остановку
    std::map<std::string_view, const Stop*> stopname_to_stop_; 

    std::deque<Bus> buses_;
    std::map<std::string_view, const Bus*> busname_to_bus_;

    std::deque<BusStat> bus_infos_;
    std::unordered_map<std::string_view, const BusStat*> busname_to_businfo_;

    std::unordered_map<const Stop*, std::set<const Bus*>> stops_to_buses_;
    
    //контейнер длин между остановками
    std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, RouteDistanceHasher> intervals_to_distance_;
    
    uint64_t GetDistance(std::pair<const Stop*, const Stop*> p) const;
    
    BusStat ComputeBusInfo(std::string_view name) const;

    friend class TransportRouter;
    
    
    // переменные счетчики для индексации
    // существуют в пределах экземпляра
    int stop_count_ = 0;
    int bus_count_ = 0;

    // добавить словарь id -> pointer ?
};  
} // namespace catalogue
