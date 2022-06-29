module;

export module fd.valve.animation_layer;

struct animation_layer
{
    bool client_blend;           // 0x00
    float blend_in;              // 0x04
    void* studio_hdr;            // 0x08
    int dispatched_src;          // 0x0C
    int dispatched_dst;          // 0x10
    int order;                   // 0x14
    int sequence;                // 0x18
    float prev_cycle;            // 0x1C
    float weight;                // 0x20
    float weight_delta_rate;     // 0x24
    float playback_rate;         // 0x28
    float cycle;                 // 0x2C
    void* owner;                 // 0x30
    int invalidate_physics_bits; // 0x34
};

export namespace fd::valve
{
    using ::animation_layer;
}
