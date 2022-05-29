
#include "geo.h"

namespace geo
{
    bool Coordinates::operator==(const Coordinates& other) const
    {
        return lat == other.lat && lng == other.lng;
    }

    bool Coordinates::operator!=(const Coordinates& other) const
    {
        return !(*this == other);
    }

    Coordinates operator+(const Coordinates& lhs, const Coordinates& rhs)
    {
        return Coordinates{ (lhs.lat + rhs.lat),(lhs.lng + rhs.lng) };
    }
}