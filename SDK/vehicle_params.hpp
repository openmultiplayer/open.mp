#pragma once

#include <types.hpp>

struct VehicleParams {
	int8_t engine = -1;
	int8_t lights = -1;
	int8_t alarm = -1;
	int8_t doors = -1;
	int8_t bonnet = -1;
	int8_t boot = -1;
	int8_t objective = -1;
	int8_t siren = -1;
	int8_t doorDriver = -1;
	int8_t doorPassenger = -1;
	int8_t doorBackLeft = -1;
	int8_t doorBackRight = -1;
	int8_t windowDriver = -1;
	int8_t windowPassenger = -1;
	int8_t windowBackLeft = -1;
	int8_t windowBackRight = -1;

	bool isSet() {
		return engine != -1 && lights != -1 && alarm != -1 && doors != -1 && bonnet != -1 && boot != -1 && objective != -1 && siren != -1 && doorDriver != -1
			&& doorPassenger != -1 && doorBackLeft != -1 && doorBackRight != -1 && windowDriver != -1 && windowPassenger != -1 && windowBackLeft != -1 && windowBackRight != -1;
	}
};