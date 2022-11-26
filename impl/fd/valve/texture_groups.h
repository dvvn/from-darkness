#pragma once

#include <fd/string.h>

namespace fd::valve
{
    // These are given to FindMaterial to reference the texture groups that Show up on the
    constexpr string_view LIGHTMAP                    = "Lightmaps";
    constexpr string_view WORLD                       = "World textures";
    constexpr string_view MODEL                       = "Model textures";
    constexpr string_view VGUI                        = "VGUI textures";
    constexpr string_view PARTICLE                    = "Particle textures";
    constexpr string_view DECAL                       = "Decal textures";
    constexpr string_view SKYBOX                      = "SkyBox textures";
    constexpr string_view CLIENT_EFFECTS              = "ClientEffect textures";
    constexpr string_view OTHER                       = "Other textures";
    constexpr string_view PRECACHED                   = "Precached";
    constexpr string_view CUBE_MAP                    = "CubeMap textures";
    constexpr string_view RENDER_TARGET               = "RenderTargets";
    constexpr string_view UNACCOUNTED                 = "Unaccounted textures";
    // constexpr string_view STATIC_VERTEX_BUFFER	=	  "Static Vertex";
    constexpr string_view STATIC_INDEX_BUFFER         = "Static Indices";
    constexpr string_view STATIC_VERTEX_BUFFER_DISP   = "Displacement Verts";
    constexpr string_view STATIC_VERTEX_BUFFER_COLOR  = "Lighting Verts";
    constexpr string_view STATIC_VERTEX_BUFFER_WORLD  = "World Verts";
    constexpr string_view STATIC_VERTEX_BUFFER_MODELS = "Model Verts";
    constexpr string_view STATIC_VERTEX_BUFFER_OTHER  = "Other Verts";
    constexpr string_view DYNAMIC_INDEX_BUFFER        = "Dynamic Indices";
    constexpr string_view DYNAMIC_VERTEX_BUFFER       = "Dynamic Verts";
    constexpr string_view DEPTH_BUFFER                = "DepthBuffer";
    constexpr string_view VIEW_MODEL                  = "ViewModel";
    constexpr string_view PIXEL_SHADERS               = "Pixel Shaders";
    constexpr string_view VERTEX_SHADERS              = "Vertex Shaders";
    constexpr string_view RENDER_TARGET_SURFACE       = "RenderTarget Surfaces";
    constexpr string_view MORPH_TARGETS               = "Morph Targets";
} // namespace fd::valve