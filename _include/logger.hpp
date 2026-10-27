#pragma once
#include "sensors.hpp" // Needs to know what a TelemetryPacket is!

// Mounts the SD card and opens a new log file (e.g., "flight_log.csv")
bool init_sd_card();

// Writes a single telemetry packet into the file buffer
void log_telemetry_packet(const TelemetryPacket &pkt);

// Forces the SD card to save buffered data to disk (cal    l sparingly during flight)
void flush_sd_card();

// Safely closes the file when the flight is over
void close_sd_card();
