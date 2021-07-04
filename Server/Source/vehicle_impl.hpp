#pragma once

#include <values.hpp>
#include <types.hpp>
#include <player.hpp>
#include <vehicle.hpp>
#include <network.hpp>
#include <events.hpp>
#include <pool.hpp>

struct Vehicle final : public IVehicle, public PoolIDProvider {
    Vector3 pos;
    GTAQuat rot;
    std::array<IPlayer*, MAX_SEATS> passengers;
    int virtualWorld_ = 0;

    virtual int getVirtualWorld() const override {
        return virtualWorld_;
    }

    virtual void setVirtualWorld(int vw) override {
        virtualWorld_ = vw;
    }

    int getID() const override {
        return poolID;
    }

    Vector3 getPosition() const override {
        return pos;
    }

    void setPosition(Vector3 position) override {
        pos = position;
    }

    GTAQuat getRotation() const override {
        return rot;
    }

    void setRotation(GTAQuat rotation) override {
        rot = rotation;
    }

    const std::array<IPlayer*, MAX_SEATS>& getPassengers() override {
        return passengers;
    }
};

struct VehiclePool : public InheritedDefaultEventDispatcherPool<Vehicle, IVehiclePool> {
    VehiclePool() {
        modelCount.fill(0u);
    }

    std::array<uint8_t, MAX_VEHICLE_MODELS>& models() override {
        return modelCount;
    }

    std::array<uint8_t, MAX_VEHICLE_MODELS> modelCount;
};
