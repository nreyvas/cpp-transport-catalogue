#pragma once

#include <cmath>

namespace geo
{
    const int EARTH_RADIUS = 6371000;

    struct Coordinates
    {
        double lat = 0.0;
        double lng = 0.0;
        bool operator==(const Coordinates& other) const;
        bool operator!=(const Coordinates& other) const;
    };

    Coordinates operator+(const Coordinates& lhs, const Coordinates& rhs);

    inline double ComputeDistance(Coordinates from, Coordinates to)
    {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = 3.1415926535 / 180.;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * EARTH_RADIUS;
    }
}