#pragma once

#include <optional>
#include <unordered_set>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <memory>

#include "transport_catalogue.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"

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
        BaseStopRequest(RequestType t, std::string n, geo::Coordinates c, std::map<std::string, int> rd);
        geo::Coordinates coords;
        std::map<std::string, int> road_distances;
    };

    struct BaseBusRequest : BaseRequest
    {
        BaseBusRequest(RequestType t, std::string n, std::vector<std::string> s, bool ir);
        std::vector<std::string> stops;
        bool is_roundtrip;
    };
    
    //---------------------------------------------------------------------------

    struct StatRequest
    {
        StatRequest(int no, RequestType t, std::string n, std::string from, std::string to);
        int id;
        RequestType type;
        std::string name;
        std::string stop_from;
        std::string stop_to;
    };
    
    class RequestHandler {
    private:

        TransportCatalogue& catalogue_;
        const renderer::MapRenderer& renderer_;
        router::TransportRouter router_;

    public:

        RequestHandler(TransportCatalogue& catalogue, const renderer::MapRenderer& renderer);

        void AddStop(BaseStopRequest& inquiry);

        void AddDistance(std::string stop_from, std::string stop_to, int distance);

        void AddBus(BaseBusRequest& inquiry);

        BusInfo GetBusInfo(StatRequest& request) const;

        StopInfo GetStopInfo(StatRequest& request) const;

        RouteInfo GetRouteInfo(StatRequest& request) const;

        svg::Document RenderMap() const;

        void SetRouterSettings(int wait_time, double bus_velocity);

        void BuildRouter();

        void ActivateRouter();

    private:

        std::vector<svg::Polyline> GenerateBuses(const renderer::SphereProjector& proj) const;
        std::vector<svg::Text> GenerateBusNames(const renderer::SphereProjector& proj) const;
        std::vector<svg::Circle> GenerateStops(const renderer::SphereProjector& proj) const;
        std::vector<svg::Text> GenerateStopNames(const renderer::SphereProjector& proj) const;

    };

    void ProcessBaseRequests(std::vector<std::unique_ptr<BaseRequest>> requests,
        RequestHandler& handler);

    std::vector<std::unique_ptr<Info>> ProcessStatRequests
        (std::vector<std::unique_ptr<StatRequest>> requests, RequestHandler& handler);
}