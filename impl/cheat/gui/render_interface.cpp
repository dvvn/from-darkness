module;

#include <cheat/core/object.h>

#include <nstd/runtime_assert.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/RenderInterface.h>

#include <DirectXMath.h>
#include <d3d9.h>

module cheat.gui.render_interface;

void custom_render_interface::ReleaseTextures()
{
    Rml::ReleaseCompiledGeometry();
}

struct DECLSPEC_NOVTABLE d3d_device_wrapped : IDirect3DDevice9
{
    d3d_device_wrapped() = delete;

    /* COM_DECLSPEC_NOTHROW HRESULT SetTransform(const D3DTRANSFORMSTATETYPE state, const D3DMATRIX* const mat)
    {
        return reinterpret_cast<IDirect3DDevice9*>(this)->SetTransform(state, mat);
    } */

    COM_DECLSPEC_NOTHROW HRESULT SetTransform(const D3DTRANSFORMSTATETYPE state, const DirectX::XMMATRIX& mat)
    {
        return reinterpret_cast<IDirect3DDevice9*>(this)->SetTransform(state, (const D3DMATRIX*)std::addressof(mat));
    }

    COM_DECLSPEC_NOTHROW HRESULT SetTransform(const D3DTRANSFORMSTATETYPE state, const Rml::ColumnMajorMatrix4f& mat)
    {
        return reinterpret_cast<IDirect3DDevice9*>(this)->SetTransform(state, (const D3DMATRIX*)mat.data());
    }

    COM_DECLSPEC_NOTHROW HRESULT SetTransform(const D3DTRANSFORMSTATETYPE state, const Rml::RowMajorMatrix4f& mat)
    {
        return this->SetTransform(state, mat.Transpose());
    }
};

namespace Rml
{
    class RenderInterfaceD3d9 final : public custom_render_interface
    {
        d3d_device_wrapped* d3d_;

      public:
        ~RenderInterfaceD3d9();
        RenderInterfaceD3d9();

        void SetupRender(Context* const ctx);
        void RenderContext(Context* const ctx) override;

        void RenderGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, TextureHandle texture, const Vector2f& translation) override;
        CompiledGeometryHandle CompileGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, TextureHandle texture) override;
        void RenderCompiledGeometry(CompiledGeometryHandle geometry, const Vector2f& translation) override;
        void ReleaseCompiledGeometry(CompiledGeometryHandle geometry) override;
        void EnableScissorRegion(bool enable) override;
        void SetScissorRegion(int x, int y, int width, int height) override;
        bool LoadTexture(TextureHandle& texture_handle, Vector2i& texture_dimensions, const String& source) override;
        bool GenerateTexture(TextureHandle& texture_handle, const byte* source, const Vector2i& source_dimensions) override;
        void ReleaseTexture(TextureHandle texture) override;
        void SetTransform(const Matrix4f* transform) override;
    };
} // namespace Rml

using Rml::RenderInterfaceD3d9;

CHEAT_OBJECT_BIND(custom_render_interface, render_interface, RenderInterfaceD3d9);
CHEAT_OBJECT_BIND(render_interface_base, render_interface_raw, RenderInterfaceD3d9);

RenderInterfaceD3d9::~RenderInterfaceD3d9()
{
    // basically unwanted, added because we init objects in random order
    Rml::ReleaseTextures(this);
}

RenderInterfaceD3d9::RenderInterfaceD3d9()
{
    const auto d3d = &CHEAT_OBJECT_GET(IDirect3DDevice9);
    d3d_ = reinterpret_cast<d3d_device_wrapped*>(d3d);
}

void RenderInterfaceD3d9::SetupRender(Context* const ctx)
{
    /*float L = clientRect.left + 0.5f;
    float R = clientRect.left + clientRect.right + 0.5f;
    float T = clientRect.top + 0.5f;
    float B = clientRect.bottom + clientRect.top + 0.5f;*/

    const auto dimensions = ctx->GetDimensions();
    // Set up an orthographic projection.
    const auto projection = DirectX::XMMatrixOrthographicOffCenterLH(0, dimensions.x + 0.5f, dimensions.y + 0.5f, 0, -1, 1);
    const auto identity = DirectX::XMMatrixIdentity();
    d3d_->SetTransform(D3DTS_WORLD, identity);
    d3d_->SetTransform(D3DTS_VIEW, identity);
    d3d_->SetTransform(D3DTS_PROJECTION, projection);

    // Switch to clockwise culling instead of counter-clockwise culling; Rocket generates counter-clockwise geometry,
    // so you can either reverse the culling mode when Rocket is rendering, or reverse the indices in the render
    // interface.
    d3d_->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

    // Enable alpha-blending for Rocket.
    d3d_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    d3d_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3d_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    // Set up the texture stage states for the diffuse texture.
    d3d_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    d3d_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    d3d_->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    d3d_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    d3d_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

    d3d_->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    d3d_->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    // Disable lighting for Rocket.
    d3d_->SetRenderState(D3DRS_LIGHTING, FALSE);
}

void RenderInterfaceD3d9::RenderContext(Context* const ctx)
{
    // Update the context to reflect any changes resulting from input events, animations,
    // modified and added elements, or changed data in data bindings.
    ctx->Update();

    [[maybe_unused]] const auto bg = d3d_->BeginScene();
    RMLUI_ASSERT(bg == D3D_OK);

    this->SetupRender(ctx);
    // Render the user interface. All geometry and other rendering commands are now
    // submitted through the render interface.
    ctx->Render();

    [[maybe_unused]] const auto ed = d3d_->EndScene();
    RMLUI_ASSERT(ed == D3D_OK);
}

// Set to byte packing, or the compiler will expand our struct, which means it won't read correctly from file
#pragma pack(push, 1)

// This structure is created for each set of geometry that Rocket compiles. It stores the vertex and index buffers and
// the texture associated with the geometry, if one was specified.
struct RocketD3D9CompiledGeometry
{
    LPDIRECT3DVERTEXBUFFER9 vertices;
    DWORD num_vertices;

    LPDIRECT3DINDEXBUFFER9 indices;
    DWORD num_primitives;

    LPDIRECT3DTEXTURE9 texture;
};

// The internal format of the vertex we use for rendering Rocket geometry. We could optimise space by having a second
// untextured vertex for use when rendering coloured borders and backgrounds.
struct RocketD3D9Vertex
{
    FLOAT x, y, z;
    DWORD colour;
    FLOAT u, v;
};

struct TGAHeader
{
    char idLength;
    char colourMapType;
    char dataType;
    short int colourMapOrigin;
    short int colourMapLength;
    char colourMapDepth;
    short int xOrigin;
    short int yOrigin;
    short int width;
    short int height;
    char bitsPerPixel;
    char imageDescriptor;
};

#pragma pack(pop)

constexpr DWORD vertex_fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

void RenderInterfaceD3d9::RenderGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, TextureHandle texture, const Vector2f& translation)
{
    // @TODO We've chosen to not support non-compiled geometry in the DirectX renderer. If you wanted to render non-compiled
    // geometry, for example for very small sections of geometry, you could use DrawIndexedPrimitiveUP or write to a
    // dynamic vertex buffer which is flushed when either the texture changes or compiled geometry is drawn.

    /*if(d3d_ == NULL)
    {
        return;
    }*/

    /// @TODO, HACK, just use the compiled geometry framework for now, this is inefficient but better than absolutely nothing
    /// for the time being
    CompiledGeometryHandle geom = this->CompileGeometry(vertices, num_vertices, indices, num_indices, texture);
    this->RenderCompiledGeometry(geom, translation);
    this->ReleaseCompiledGeometry(geom);
}

auto RenderInterfaceD3d9::CompileGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, TextureHandle texture) -> CompiledGeometryHandle
{
    // Construct a new RocketD3D9CompiledGeometry structure, which will be returned as the handle, and the buffers to
    // store the geometry.

    RocketD3D9CompiledGeometry* geometry = new RocketD3D9CompiledGeometry();
    d3d_->CreateVertexBuffer(num_vertices * sizeof(RocketD3D9Vertex), D3DUSAGE_WRITEONLY, vertex_fvf, D3DPOOL_DEFAULT, &geometry->vertices, NULL);
    d3d_->CreateIndexBuffer(num_indices * sizeof(unsigned int), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &geometry->indices, NULL);

    // Fill the vertex buffer.
    RocketD3D9Vertex* d3d9_vertices;
    geometry->vertices->Lock(0, 0, (void**)&d3d9_vertices, 0);
    for (int i = 0; i < num_vertices; ++i)
    {
        d3d9_vertices[i].x = vertices[i].position.x;
        d3d9_vertices[i].y = vertices[i].position.y;
        d3d9_vertices[i].z = 0;

        d3d9_vertices[i].colour = D3DCOLOR_RGBA(vertices[i].colour.red, vertices[i].colour.green, vertices[i].colour.blue, vertices[i].colour.alpha);

        d3d9_vertices[i].u = vertices[i].tex_coord[0];
        d3d9_vertices[i].v = vertices[i].tex_coord[1];
    }
    geometry->vertices->Unlock();

    // Fill the index buffer.
    unsigned int* d3d9_indices;
    geometry->indices->Lock(0, 0, (void**)&d3d9_indices, 0);
    memcpy(d3d9_indices, indices, sizeof(unsigned int) * num_indices);
    geometry->indices->Unlock();

    geometry->num_vertices = (DWORD)num_vertices;
    geometry->num_primitives = (DWORD)num_indices / 3;

    geometry->texture = /*texture == NULL ? NULL :*/ (LPDIRECT3DTEXTURE9)texture;

    return (CompiledGeometryHandle)geometry;
}

void RenderInterfaceD3d9::RenderCompiledGeometry(CompiledGeometryHandle geometry, const Vector2f& translation)
{
    // Build and set the transform matrix.
    auto world_transform = DirectX::XMMatrixTranslation(translation.x, translation.y, 0);
    d3d_->SetTransform(D3DTS_WORLD, world_transform);

    RocketD3D9CompiledGeometry* d3d9_geometry = (RocketD3D9CompiledGeometry*)geometry;

    // Set the vertex format for the Rocket vertices, and bind the vertex and index buffers.
    d3d_->SetFVF(vertex_fvf);
    d3d_->SetStreamSource(0, d3d9_geometry->vertices, 0, sizeof(RocketD3D9Vertex));
    d3d_->SetIndices(d3d9_geometry->indices);

    // Set the texture, if this geometry has one.
    if (d3d9_geometry->texture != NULL)
        d3d_->SetTexture(0, d3d9_geometry->texture);
    else
        d3d_->SetTexture(0, NULL);

    // Draw the primitives.
    d3d_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, d3d9_geometry->num_vertices, 0, d3d9_geometry->num_primitives);
}

void RenderInterfaceD3d9::ReleaseCompiledGeometry(CompiledGeometryHandle geometry)
{
    RocketD3D9CompiledGeometry* d3d9_geometry = (RocketD3D9CompiledGeometry*)geometry;

    d3d9_geometry->vertices->Release();
    d3d9_geometry->indices->Release();

    delete d3d9_geometry;
}

void RenderInterfaceD3d9::EnableScissorRegion(bool enable)
{
    d3d_->SetRenderState(D3DRS_SCISSORTESTENABLE, enable);
}

void RenderInterfaceD3d9::SetScissorRegion(int x, int y, int width, int height)
{
    RECT scissor_rect;
    scissor_rect.left = x;
    scissor_rect.right = x + width;
    scissor_rect.top = y;
    scissor_rect.bottom = y + height;

    d3d_->SetScissorRect(&scissor_rect);
}

bool RenderInterfaceD3d9::LoadTexture(TextureHandle& texture_handle, Vector2i& texture_dimensions, const String& source)
{
    FileInterface* file_interface = GetFileInterface();
    FileHandle file_handle = file_interface->Open(source);
    if (file_handle == NULL)
        return false;

    file_interface->Seek(file_handle, 0, SEEK_END);
    size_t buffer_size = file_interface->Tell(file_handle);
    file_interface->Seek(file_handle, 0, SEEK_SET);

    char* buffer = new char[buffer_size];
    file_interface->Read(buffer, buffer_size, file_handle);
    file_interface->Close(file_handle);

    TGAHeader header;
    memcpy(&header, buffer, sizeof(TGAHeader));

    int color_mode = header.bitsPerPixel / 8;
    int image_size = header.width * header.height * 4; // We always make 32bit textures

    if (header.dataType != 2)
    {
        Log::Message(Log::LT_ERROR, "Only 24/32bit uncompressed TGAs are supported.");
        return false;
    }

    // Ensure we have at least 3 colors
    if (color_mode < 3)
    {
        Log::Message(Log::LT_ERROR, "Only 24 and 32bit textures are supported");
        return false;
    }

    const char* image_src = buffer + sizeof(TGAHeader);
    unsigned char* image_dest = new unsigned char[image_size];

    // Targa is BGR, swap to RGB and flip Y axis
    for (long y = 0; y < header.height; y++)
    {
        long read_index = y * header.width * color_mode;
        long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (header.height - y - 1) * header.width * color_mode;
        for (long x = 0; x < header.width; x++)
        {
            image_dest[write_index] = image_src[read_index + 2];
            image_dest[write_index + 1] = image_src[read_index + 1];
            image_dest[write_index + 2] = image_src[read_index];
            if (color_mode == 4)
                image_dest[write_index + 3] = image_src[read_index + 3];
            else
                image_dest[write_index + 3] = 255;

            write_index += 4;
            read_index += color_mode;
        }
    }

    texture_dimensions.x = header.width;
    texture_dimensions.y = header.height;

    bool success = GenerateTexture(texture_handle, image_dest, texture_dimensions);

    delete[] image_dest;
    delete[] buffer;

    return success;
}

bool RenderInterfaceD3d9::GenerateTexture(TextureHandle& texture_handle, const byte* source, const Vector2i& source_dimensions)
{
    // Create a Direct3DTexture9, which will be set as the texture handle. Note that we only create one surface for this texture;
    // because we're rendering in a 2D context, mip-maps are not required.
    LPDIRECT3DTEXTURE9 d3d9_texture;
    if (d3d_->CreateTexture(source_dimensions.x, source_dimensions.y, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &d3d9_texture, NULL) != D3D_OK)
        return false;

    // Lock the top surface and write the pixel data onto it.
    D3DLOCKED_RECT locked_rect;
    d3d9_texture->LockRect(0, &locked_rect, NULL, 0);
    for (int y = 0; y < source_dimensions.y; ++y)
    {
        for (int x = 0; x < source_dimensions.x; ++x)
        {
            const byte* source_pixel = source + (source_dimensions.x * 4 * y) + (x * 4);
            byte* destination_pixel = ((byte*)locked_rect.pBits) + locked_rect.Pitch * y + x * 4;

            destination_pixel[0] = source_pixel[2];
            destination_pixel[1] = source_pixel[1];
            destination_pixel[2] = source_pixel[0];
            destination_pixel[3] = source_pixel[3];
        }
    }
    d3d9_texture->UnlockRect(0);

    // Set the handle on the Rocket texture structure.
    texture_handle = (TextureHandle)d3d9_texture;
    return true;
}

void RenderInterfaceD3d9::ReleaseTexture(TextureHandle texture_handle)
{
    ((LPDIRECT3DTEXTURE9)texture_handle)->Release();
}

void RenderInterfaceD3d9::SetTransform(const Matrix4f* transform)
{
    if (!transform)
        d3d_->SetTransform(D3DTS_VIEW, DirectX::XMMatrixIdentity());
    else
        d3d_->SetTransform(D3DTS_VIEW, *transform);
}
