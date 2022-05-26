#pragma once

#include <optional>
#include <unordered_set>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <memory>

#include "transport_catalogue.h"

namespace transport_catalogue
{


    struct BaseRequest
    {
        BaseRequest(RequestType t, std::string n);
        RequestType type;
        std::string name;
    };

    struct BaseStopRequest : BaseRequest
    {
        BaseStopRequest(RequestType t, std::string n, Coordinates c, std::map<std::string, int> rd);
        Coordinates coords;
        std::map<std::string, int> road_distances;
    };

    struct BaseBusRequest : BaseRequest
    {
        BaseBusRequest(RequestType t, std::string n, std::vector<std::string> s, bool ir);
        std::vector<std::string> stops;
        bool is_roundtrip;
    };

    //---------------------------------------------------------------------------
        
    class BaseRequestHandler
    {
    public:
        BaseRequestHandler(TransportCatalogue& catalogue)
            : catalogue_(catalogue) {}

        void AddStop(BaseStopRequest& inquiry);

        void AddDistance(std::string stop_from, std::string stop_to, int distance);

        void AddBus(BaseBusRequest& inquiry);
        
    private:

        TransportCatalogue& catalogue_;
    };

    void ProcessBaseRequests(std::vector<std::unique_ptr<BaseRequest>> requests,
        BaseRequestHandler handler);
    
    
    //---------------------------------------------------------------------------

    struct StatRequest
    {
        StatRequest(int no, RequestType t, std::string n);
        int id;
        RequestType type;
        std::string name;
    };
    
    class StatRequestHandler {
    public:

        StatRequestHandler(const TransportCatalogue& catalogue);

        BusInfo GetBusInfo(StatRequest& request) const;

        StopInfo GetStopInfo(StatRequest& request) const;

        //RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

        //std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        //const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

        //svg::Document RenderMap() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& catalogue_;
        //const renderer::MapRenderer& renderer_;
    };

    std::vector<std::unique_ptr<Info>> ProcessStatRequests
        (std::vector<std::unique_ptr<StatRequest>> requests, StatRequestHandler handler);
}