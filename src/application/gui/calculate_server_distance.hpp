#pragma once
#include <cmath>
#include <unordered_map>
#include <string>
#include "augs/misc/mutex.h"
#include <optional>

// Define constants
const double EARTH_RADIUS_KM = 6371.0;
const double SPEED_OF_LIGHT_KM_PER_MS = 299.792458; // Speed of light in km/ms

// Structure to hold latitude and longitude
struct lat_long {
    double latitude;
    double longitude;
};

// Unordered map to hold the mappings
std::unordered_map<std::string, lat_long> server_locations = {
    {"au", { -33.8688, 151.2093 }},       // Sydney, Australia
    {"ru", { 55.7558, 37.6173 }},         // St. Petersburg, Russia
    {"de", { 52.5200, 13.4050 }},         // Berlin, Germany
    {"us-central", { 41.8781, -87.6298 }},// Chicago, USA
    {"pl", { 52.2297, 21.0122 }},         // Warsaw, Poland
    {"ch", { 47.3769, 8.5417 }},          // Zurich, Switzerland
    {"nl", { 50.8746, 6.0580 }},           // Eygelshoven, Netherlands
    {"fi", { 60.1695, 24.9354 }}          // Helsinki, Finland
};

// Function to convert degrees to radians
double degrees_to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

// Function to calculate the distance between two points given their latitude and longitude using the Haversine formula
double calculate_distance(double lat1, double lon1, double lat2, double lon2) {
    double d_lat = degrees_to_radians(lat2 - lat1);
    double d_lon = degrees_to_radians(lon2 - lon1);

    double a = sin(d_lat / 2) * sin(d_lat / 2) +
               cos(degrees_to_radians(lat1)) * cos(degrees_to_radians(lat2)) *
               sin(d_lon / 2) * sin(d_lon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS_KM * c;
}

// Function to approximate ping time based on distance
double calculate_ping(double distance) {
    // Approximate ping time is distance divided by the speed of light
    const double quality_mult = 4;
    return (distance / SPEED_OF_LIGHT_KM_PER_MS) * quality_mult * 2; // Multiply by 2 to account for round trip
}

augs::mutex lat_lon_mutex;
std::optional<double> player_latitude;
std::optional<double> player_longitude;

std::optional<double> get_estimated_ping_to_server(const std::string& loc_id) {
    if (const auto latlon = server_locations.find(loc_id); latlon != server_locations.end()) {
        double server_lat = latlon->second.latitude;
        double server_lon = latlon->second.longitude;

        auto lock = augs::scoped_lock(lat_lon_mutex);

        if (player_latitude.has_value() && player_longitude.has_value()) {
            const auto ping = calculate_ping(calculate_distance(*player_latitude, *player_longitude, server_lat, server_lon));
            return ping;
        }
    }

    return std::nullopt;
}
