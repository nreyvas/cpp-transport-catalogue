#include "transport_catalogue.h"

namespace catalogue {
void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string>& stops, BusType type) {
    std::vector<const Stop*> stops_ptr;
    auto it = buses_.insert(buses_.end(), std::move(Bus{std::string(name), stops_ptr, type, bus_count_++}));
    
    for_each(stops.begin(), stops.end(), [&stops_ptr, &it, this](std::string_view stop_name){
        if(const Stop* stop_ptr = FindStop(stop_name)) { // если есть такая остановка в базе
            it->stops_.push_back(std::move(stop_ptr));
            //добавление в stops_to_buses_
            stops_to_buses_[stop_ptr].insert(&*it);
        }
        
    });
    busname_to_bus_[std::string_view{it->name_}] = &(*it);
}

void TransportCatalogue::AddStop(std::string_view name, Coordinates coordinates) {
    auto it = stops_.emplace(stops_.end(), std::move(Stop{std::string(name), coordinates, stop_count_++}));
    stopname_to_stop_[std::string_view{it->name_}] = &(*it);
}

const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    if(stopname_to_stop_.count(name) == 0) {
        return nullptr;
    }
    return stopname_to_stop_.at(name);
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    if(busname_to_bus_.count(name) == 0) {
        return nullptr;
    } 
    return busname_to_bus_.at(name);
}

BusStat TransportCatalogue::GetBusInfo(std::string_view name) const {
    if(busname_to_businfo_.count(name) == 0) {
        // статистики нет в базе
        return ComputeBusInfo(name);
    } 
    // статистика уже есть в базе
    return *(busname_to_businfo_.at(name));
}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    if(stopname_to_stop_.count(stop_name) != 0) {
        // остановка существует
        const Stop* stop_ptr = FindStop(stop_name);
        if(stops_to_buses_.count(stop_ptr) != 0) {
            //остановка есть и через нее едут автобусы
            StopInfo stop_info{{}, true};

            std::for_each(stops_to_buses_.at(stop_ptr).begin(),
                            stops_to_buses_.at(stop_ptr).end(),
                            [&](auto bus){
                                stop_info.buses_.insert(bus->name_);
                            });
            return stop_info;
        } else {
            return StopInfo{{}, true}; // остановка есть, а автобусов нет
        }
    } else {
        return StopInfo{{}, false}; // остановка не существует
    }
}

void TransportCatalogue::SetDistance(std::pair<const Stop*, const Stop*> p, uint64_t distance) {
    intervals_to_distance_[p] = distance;
    // маршрут в обратную сторону еще не задавался
    // или задается расстояние к одной и той же остановке
    if(intervals_to_distance_.count({p.second, p.first}) == 0
        || p.first == p.second) { 
        // значит, что длина маршрута в обе стороны равна
        intervals_to_distance_[{p.second, p.first}] = distance;
    }
}

uint64_t TransportCatalogue::GetDistance(std::pair<const Stop*, const Stop*> p) const {
    if(intervals_to_distance_.count(p)) {
        return intervals_to_distance_.at(p);
    } else {
        return 0u;
    }
}

BusStat TransportCatalogue::ComputeBusInfo(std::string_view name) const {
    const Bus* bus{nullptr}; //указатель на автобус в деке
    unsigned int stops_count = 0, unique_stops_count = 0;
    double length_geo = 0;
    uint64_t length_road = 0;
    double curvature = 0.0;
    if(bus = FindBus(name), bus) {
        // такой автобус существует
        stops_count = bus->stops_.size();

        std::unordered_set<std::string_view, std::hash<std::string_view>, std::equal_to<std::string_view>> unique_stops;
        for_each(bus->stops_.begin(), bus->stops_.end(), [&unique_stops](const Stop* stop){
            //std::cout << "Debug. \"" << stop->name_ << "\"." << std::endl;
            unique_stops.insert(std::string_view(stop->name_));
        });
        unique_stops_count = unique_stops.size();
        
        length_geo = std::transform_reduce(
            next(bus->stops_.begin()), bus->stops_.end(),
            bus->stops_.begin(),
            0.0,
            std::plus<>(),
            [](const Stop* s, const Stop* prev_s){
                if(s == prev_s) { return 0.0; } // между одной и той же остановкой нет расстояния
                return ComputeDistance(prev_s->cordinates_, s->cordinates_);
            }
        );

        length_road = std::transform_reduce(
            next(bus->stops_.begin()), bus->stops_.end(),
            bus->stops_.begin(),
            0ULL,
            std::plus<>(),
            [this, &bus](const Stop* s, const Stop* prev_s){
                if(bus->bus_type_ == BusType::CYCLED) {
                    return GetDistance({prev_s, s});
                } else if(bus->bus_type_ == BusType::ORDINARY) {
                    return GetDistance({prev_s, s}) + GetDistance({s, prev_s});
                } else {
                    uint64_t c = 0;
                    return c;
                }
            }
        );

        if(bus->bus_type_ == BusType::ORDINARY) { 
            length_geo *= 2;
            stops_count = stops_count * 2 - 1; 
        } //у обычного маршрута расстояние умножается на два

        curvature = static_cast<long double>(length_road) / static_cast<long double>(length_geo);


        // вариант с хранением статистики в базе каталога
        // запишем рассчитанную информацию в базу
        // auto it = bus_infos_.insert(bus_infos_.end(), std::move(BusStat{stops_count, unique_stops_count, length_road, curvature}));
        //busname_to_businfo_[name] = &*it;
        // возвращаем рассчитанный объект со статистикой
        // return *it;

        // вариант с возвратом промежуточной информации
        return BusStat{stops_count, unique_stops_count, length_road, curvature, true};
    } else {
        // несуществующий автобус
        return BusStat{};
    }
}  

void TransportCatalogue::PrintIntervals() const {
    for(const auto& e : intervals_to_distance_) {
        std::cout << "From: \"" << (e.first.first)->name_ << "\" to \"" <<  (e.first.second)->name_ << "\". = " << e.second << "m. " << std::endl;
    }
}  

std::deque<const Bus*> TransportCatalogue::GetBusesSorted() const {
    std::deque<const Bus*> result;
    for(const auto& [busname, bus] : busname_to_bus_) {
        result.push_back(bus);
    }
    return result;
}

const std::unordered_map<const Stop*, std::set<const Bus*>>& TransportCatalogue::GetStopsToBuses() const {
    return stops_to_buses_;
}

const std::map<std::string_view, const Stop*>& TransportCatalogue::GetStopnameToStops() const {
    return stopname_to_stop_;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

const std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, RouteDistanceHasher>& 
TransportCatalogue::GetIntervalsToDistance() const {
    return intervals_to_distance_;
}

void TransportCatalogue::SetStops(std::deque<Stop>&& stops) {
    stops_.swap(stops);
}

void TransportCatalogue::SetStopnameToStop(std::map<std::string_view, const Stop*>&& stopname_to_stop) {
    stopname_to_stop_ = stopname_to_stop;
}


void TransportCatalogue::SetBuses(std::deque<Bus>&& buses) {
    buses_.swap(buses);
}
void TransportCatalogue::SetBusnameToBus(std::map<std::string_view, const Bus*>&& busname_to_bus) {
    busname_to_bus_ = busname_to_bus;
}
void TransportCatalogue::SetStopsToBuses(std::unordered_map<const Stop*, std::set<const Bus*>>&& stops_to_buses) {
    stops_to_buses_ = stops_to_buses;
}
void TransportCatalogue::SetIntervalsToDistance(std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, catalogue::RouteDistanceHasher>&& intervals_to_distance) {
    intervals_to_distance_ = intervals_to_distance;
}
} // namespace catalogue

