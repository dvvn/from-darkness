#pragma once

#include <cstdint>

namespace fd::valve
{
struct global_vars_base
{
    float real_time;                     // 0x00
    uint32_t frame_count;                // 0x04
    float abs_frame_time;                // 0x08
    float abs_frame_start_time;          // 0x0C
    float current_time;                  // 0x10
    float frame_time;                    // 0x14
    uint32_t max_clients;                // 0x18
    uint32_t tick_count;                 // 0x1C
    float interval_per_tick;             // 0x20
    float interpolation_amount;          // 0x24
    uint32_t frame_simulation_ticks;     // 0x28
    uint32_t network_protocol;           // 0x2C
    void *save_data;                     // 0x30
    bool client;                         // 0x34
    bool remote_client;                  // 0x35
    uint32_t timestamp_networking_base;  // 0x38
    uint32_t timestamp_randomize_window; // 0x3C
};

struct global_vars : global_vars_base
{
    char const *map_name;       // 0x40
    char const *map_group_name; // 0x44
    uint32_t map_version;       // 0x48
    char const *start_spot;     // 0x4C
    uint32_t load_type;         // 0x50
    bool map_load_failed;       // 0x54
    bool deathmatch;            // 0x55
    bool cooperative;           // 0x56
    bool teamplay;              // 0x57
    uint32_t max_entities;      // 0x58
    uint32_t server_count;      // 0x5C
    void *edicts;               // 0x60
};
} // namespace fd::valve