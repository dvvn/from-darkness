module;

#include <RmlUi/Core/RenderInterface.h>

export module cheat.gui.render_interface;

using namespace Rml;

export namespace cheat::gui
{
	class render_interface final : public RenderInterface
	{
	public:
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

		//-------------

		void RenderContext(Context* const ctx);
		void ReleaseTextures( );
	};
}