module;

#include <nstd/runtime_assert.h>
#include <nstd/winapi/comptr.h>

#include <RmlUi/Core.h>
#ifdef _DEBUG
#include <RmlUi/Debugger.h>
#endif

#include <d3d9.h>

module cheat.gui;
import :menu;
import cheat.gui.render_interface;
import cheat.gui.system_interface;
import nstd.one_instance;

using namespace cheat;

#define RML_ASSERT_DIR NSTD_CONCAT(RMLUI_DIR, \Samples\assets\)
#define RML_SAMPLE(_S_) NSTD_CONCAT(NSTD_STRINGIZE_RAW(RML_ASSERT_DIR),_S_)

Rml::Context* nstd::one_instance_getter<Rml::Context*>::_Construct( ) const noexcept
{
	nstd::winapi::comptr<IDirect3DSurface9> back_buffer;
	nstd::get_instance<IDirect3DDevice9*>( )->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, back_buffer);

	D3DSURFACE_DESC desc;
	back_buffer->GetDesc(&desc);
	return Rml::CreateContext("main", Rml::Vector2i(desc.Width, desc.Height));
}

struct rml_loader
{
	rml_loader( )
	{
		Rml::SetRenderInterface(nstd::one_instance<gui::render_interface>::get_ptr( ));
		Rml::SetSystemInterface(nstd::one_instance<gui::system_interface>::get_ptr( ));

		Rml::Initialise( );
	}

	~rml_loader( )
	{
		Rml::Shutdown( );
	}
};

void gui::init( )
{
	(void)nstd::get_instance<rml_loader>( );
	Rml::Context* context = nstd::get_instance<Rml::Context*>( );

#ifdef _DEBUG
	Rml::Debugger::Initialise(context);
	Rml::Debugger::SetVisible(true);
#endif

	// Tell RmlUi to load the given fonts.
	Rml::LoadFontFace(RML_SAMPLE("LatoLatin-Regular.ttf"));
	// Fonts can be registered as fallback fonts, as in this case to display emojis.
	Rml::LoadFontFace(RML_SAMPLE("NotoEmoji-Regular.ttf"), true);

	Rml::LoadFontFace("C:/Windows/fonts/arial.ttf", true);

	context->LoadDocument(RML_SAMPLE("demo.rml"))->Show( );
	context->LoadDocument(RML_SAMPLE("window.rml"))->Show( );

#if 0
	// Replace and style some text in the loaded document.
	Rml::Element* element = document->GetElementById("world");
	element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
	element->SetProperty("font-size", "1.5em");

	// Set up data bindings to synchronize application data.
	if(Rml::DataModelConstructor constructor = context->CreateDataModel("animals"))
	{
		constructor.Bind("show_text", &my_data.show_text);
		constructor.Bind("animal", &my_data.animal);
	}
#endif
}

void gui::render( )
{
	auto context = nstd::get_instance<Rml::Context*>( );
	auto d3d = nstd::get_instance<IDirect3DDevice9*>( );

	// Update the context to reflect any changes resulting from input events, animations,
	// modified and added elements, or changed data in data bindings.
	context->Update( );

	[[maybe_unused]]
	const auto bg = d3d->BeginScene( );
	runtime_assert(bg == D3D_OK);

	// Render the user interface. All geometry and other rendering commands are now
	// submitted through the render interface.
	context->Render( );

	[[maybe_unused]]
	const auto ed = d3d->EndScene( );
	runtime_assert(ed == D3D_OK);
}