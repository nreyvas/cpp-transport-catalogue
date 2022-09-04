#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "svg.h"

#include <optional>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
using catalogue::TransportCatalogue;
using catalogue::BusStat;
using catalogue::StopInfo;
using BusPtr = Bus*;

class RequestHandler {
public:

    RequestHandler(const TransportCatalogue& db, 
    renderer::MapRenderer& renderer, 
    graph::Router<BusRouteWeight>& router,
    catalogue::TransportRouter& t_router);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    std::string RenderMap();

    std::optional<StopInfo> GetStopInfo(const std::string_view& bus_name) const;

    std::optional<graph::Router<BusRouteWeight>::RouteInfo> GetRouteInfo(std::string_view stop_from, std::string_view stop_to) const;


    const graph::Edge<BusRouteWeight>& GetEdgeByIndex(graph::EdgeId edge_id) const;
    const Bus* GetBusByEdgeIndex(graph::EdgeId edge_id) const;
    const Stop* GetStopByVertexIndex(graph::VertexId vertex_id) const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;

    const graph::Router<BusRouteWeight>& router_;
    const catalogue::TransportRouter& t_router_;
};


