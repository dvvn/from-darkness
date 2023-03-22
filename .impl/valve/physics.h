#pragma once

#include <cstdint>

namespace fd::valve
{
    struct surface_params
    {
        float friction;
        float elasticity;
        float density;
        float thickness;
        float dampening;
    };

    struct surface_autio_params
    {
        float reflectivity;          // like elasticity, but how much sound should be reflected by this surface
        float hardnessFactor;        // like elasticity, but only affects impact sound choices
        float roughnessFactor;       // like friction, but only affects scrape sound choices
        float roughThreshold;        // surface roughness > this causes "rough" scrapes, < this causes "smooth" scrapes
        float hardThreshold;         // surface hardness > this causes "hard" impacts, < this causes "soft" impacts
        float hardVelocityThreshold; // collision velocity > this causes "hard" impacts, < this causes "soft" impacts
        float highPitchOcclusion;    // a value betweeen 0 and 100 where 0 is not occluded at all and 100 is silent (except for any additional reflected sound)
        float midPitchOcclusion;
        float lowPitchOcclusion;
    };

    struct surface_sound_names
    {
        uint16_t walkStepLeft;
        uint16_t walkStepRight;
        uint16_t runStepLeft;
        uint16_t runStepRight;
        uint16_t impactSoft;
        uint16_t impactHard;
        uint16_t scrapeSmooth;
        uint16_t scrapeRough;
        uint16_t bulletImpact;
        uint16_t rolling;
        uint16_t breakSound;
        uint16_t strainSound;
    };

    struct surface_game_group
    {
        float maxSpeedFactor;
        float jumpFactor;
        float flPenetrationModifier;
        float flDamageModifier;
        uint16_t material;
        uint8_t climbable;
    };

    struct surface_data
    {
        surface_params physics;
        surface_autio_params audio;
        surface_sound_names sounds;
        surface_game_group game;
    };

    struct physics_surface_props
    {
        virtual ~physics_surface_props() = default;

        virtual int ParseSurfaceData(const char* pFilename, const char* pTextfile)                                                       = 0;
        virtual int SurfacePropCount() const                                                                                             = 0;
        virtual int GetSurfaceIndex(const char* pSurfacePropName) const                                                                  = 0;
        virtual void GetPhysicsProperties(int surfaceDataIndex, float* density, float thickness, float friction, float elasticity) const = 0;
        virtual surface_data* GetSurfaceData(int surfaceDataIndex)                                                                       = 0;
        virtual const char GetString(uint16_t stringTableIndex) const                                                                    = 0;
        virtual const char GetPropName(int surfaceDataIndex) const                                                                       = 0;
        virtual void SetWorldMaterialIndexTable(int* pMapArray, int mapSize)                                                             = 0;
        virtual void GetPhysicsParameters(int surfaceDataIndex, surface_params* pParamsOut) const                                        = 0;
    };
} // namespace fd::valve
