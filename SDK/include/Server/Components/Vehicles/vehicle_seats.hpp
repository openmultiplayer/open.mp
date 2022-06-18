#pragma once

#include "vehicles.hpp"
#include <types.hpp>

namespace Impl {

/// Returns 0xFF for invalid vehicles, and returns 0 for vehicles with only a driver.
inline uint8_t getVehiclePassengerSeats(int model)
{
    if (!isValidVehicleModel(model)) {
        return 255;
    }

    static const StaticArray<uint8_t, MAX_VEHICLE_MODELS> allSeats = {
          1,   0,   3,   3,   1,   1,   1,   3,   1,   1,   3,   1,   1,   1,   3,   1,
          1,   1,   3,   3,   1,   3,   1,   3,   8,   0,   1,   3,   3,   3,   0,   1,
          1,   3,   8,   1, 255,   1,   1,   0,   1,   0,   3,   1,   1,   1,   0,   3,
          1,   0,   0,   0,   1, 255,   1,   0,   1,   1,   1,   1,   3,   3,   1,   1,
          1,   3,   1,   1,   3,   3,   1,   1,   3,   1,   1,   0,   1,   1,   0,   0,
          3,   0,   0,   0,   2,   3,   0,   1,   1,   1,   0,   3,   1,   3,   1,   1,
          1,   1,   1,   1,   1,   3,   3,   1,   1,   0,   0,   1,   3,   1,   1,   1,
          0,   1,   1,   3,   1,   1,   0,   0,   1,   1,   1,   1,   1,   1,   1,   0,
          1,   1,   1,   0,   0,   0,   3,   1,   1,   1,   1,   3,   0,   1,   1,   1,
          3,   3,   1,   1,   3,   3,   1,   1,   1,   1,   1,   1,   1,   1,   0,   1,
          3,   3,   1,   0,   1,   1,   3,   3,   1,   0,   1,   0,   0,   5,   1,   0,
          0,   3,   1,   3,   3,   1,   1,   1, 255, 255,   1,   1,   1,   1,   3, 255,
          1,   3,   3,   3,   0,   1,   1,   1, 255, 255,   1,   3,   1,   1,   1,   1,
          0,   0,   0,   0,
	};

    return allSeats[model - 400];
}

}

