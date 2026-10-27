#pragma once

enum class FlightState {
    GROUND_IDLE,
    POWERED,
    UNPOWERED,
    CHUTE,
    LANDED
};

FlightState update_flight_state(float alt, float accel_z, FlightState state);
