#pragma once

#include <values.hpp>
#include <types.hpp>
#include <player.hpp>
#include <vehicle.hpp>
#include <network.hpp>
#include <events.hpp>
#include <pool.hpp>

struct Vehicle final : public IVehicle, public PoolIDProvider {
    vector3 pos;
    vector4 rot;
    std::array<IPlayer*, MAX_SEATS> passengers;

    int getID() override {
        return poolID;
    }

    vector3 getPosition() override {
        return pos;
    }

    void setPosition(vector3 position) override {
        pos = position;
    }

    vector4 getRotation() override {
        return rot;
    }

    void setRotation(vector4 rotation) override {
        rot = rotation;
    }

    const std::array<IPlayer*, MAX_SEATS>& getPassengers() override {
        return passengers;
    }
};

struct VehiclePool : public InheritedEventDispatcherPool<Vehicle, IVehiclePool> {
    VehiclePool() {
        modelCount.fill(0u);
    }

    std::array<uint8_t, MAX_VEHICLE_MODELS>& models() override {
        return modelCount;
    }

    std::array<uint8_t, MAX_VEHICLE_MODELS> modelCount;
};
