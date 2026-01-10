#define _USE_MATH_DEFINES
#include "geo.h"
#include <cmath>

namespace geo {

    double ComputeDistance(Coordinates from, Coordinates to) {
        static const double dr = M_PI / 180.0;
        static const int kEarthRadius = 6371000;

        return std::acos(
            std::sin(from.lat * dr) * std::sin(to.lat * dr) +
            std::cos(from.lat * dr) * std::cos(to.lat * dr) *
            std::cos(std::abs(from.lng - to.lng) * dr)
        ) * kEarthRadius;
    }

}  // namespace geo
