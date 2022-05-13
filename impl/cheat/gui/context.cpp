module;

#include <cheat/tools/interface.h>

#include <nstd/core_utils.h>

#include <RmlUi/Core.h>
#ifdef _DEBUG
#include <RmlUi/Debugger.h>
#endif

#include <memory>

module cheat.gui.context;
import cheat.gui.render_interface;
import cheat.gui.system_interface;
import cheat.tools.window_info;

using namespace cheat;
using namespace gui;
using namespace Rml;

#define RML_SAMPLES NSTD_CONCAT(RMLUI_DIR, \Samples\)
#define RML_SAMPLE(_DIR_,_S_) NSTD_STRINGIZE_RAW(RML_SAMPLES)##_DIR_##"\\"##_S_

static void _Rml_demo_animation(Context* ctx)
{
    auto document = ctx->LoadDocument(RML_SAMPLE("basic\\animation\\data", "animation.rml"));
    document->GetElementById("title")->SetInnerRML("animation demo");

    // Button fun
    {
        auto el = document->GetElementById("start_game");
        auto p1 = Transform::MakeProperty({Transforms::Rotate2D{10.f}, Transforms::TranslateX{100.f}});
        auto p2 = Transform::MakeProperty({Transforms::Scale2D{3.f}});
        el->Animate("transform", p1, 1.8f, Tween{Tween::Elastic, Tween::InOut}, -1, true);
        el->AddAnimationKey("transform", p2, 1.3f, Tween{Tween::Elastic, Tween::InOut});
    }
    {
        auto el = document->GetElementById("high_scores");
        el->Animate("margin-left", Property(0.f, Property::PX), 0.3f, Tween{Tween::Sine, Tween::In}, 10, true, 1.f);
        el->AddAnimationKey("margin-left", Property(100.f, Property::PX), 3.0f, Tween{Tween::Circular, Tween::Out});
    }
    {
        auto el = document->GetElementById("options");
        el->Animate("image-color", Property(Colourb(128, 255, 255, 255), Property::COLOUR), 0.3f, Tween{}, -1, false);
        el->AddAnimationKey("image-color", Property(Colourb(128, 128, 255, 255), Property::COLOUR), 0.3f);
        el->AddAnimationKey("image-color", Property(Colourb(0, 128, 128, 255), Property::COLOUR), 0.3f);
        el->AddAnimationKey("image-color", Property(Colourb(64, 128, 255, 0), Property::COLOUR), 0.9f);
        el->AddAnimationKey("image-color", Property(Colourb(255, 255, 255, 255), Property::COLOUR), 0.3f);
    }
    {
        auto el = document->GetElementById("exit");
        PropertyDictionary pd;
        StyleSheetSpecification::ParsePropertyDeclaration(pd, "transform", "translate(200px, 200px) rotate(1215deg)");
        el->Animate("transform", *pd.GetProperty(PropertyId::Transform), 3.f, Tween{Tween::Bounce, Tween::Out}, -1);
    }

    // Transform tests
    {
        auto el = document->GetElementById("generic");
        auto p = Transform::MakeProperty({Transforms::TranslateY{50, Property::PX}, Transforms::Rotate3D{0, 0, 1, -90, Property::DEG}, Transforms::ScaleY{0.8f}});
        el->Animate("transform", p, 1.5f, Tween{Tween::Sine, Tween::InOut}, -1, true);
    }
    {
        auto el = document->GetElementById("combine");
        auto p = Transform::MakeProperty({Transforms::Translate2D{50, 50, Property::PX}, Transforms::Rotate2D(1215)});
        el->Animate("transform", p, 8.0f, Tween{}, -1, true);
    }
    {
        auto el = document->GetElementById("decomposition");
        auto p = Transform::MakeProperty({Transforms::TranslateY{50, Property::PX}, Transforms::Rotate3D{0.8f, 0, 1, 110, Property::DEG}});
        el->Animate("transform", p, 1.3f, Tween{Tween::Quadratic, Tween::InOut}, -1, true);
    }

    // Mixed units tests
    {
        auto el = document->GetElementById("abs_rel");
        el->Animate("margin-left", Property(50.f, Property::PERCENT), 1.5f, Tween{}, -1, true);
    }
    {
        auto el = document->GetElementById("abs_rel_transform");
        auto p = Transform::MakeProperty({Transforms::TranslateX{0, Property::PX}});
        el->Animate("transform", p, 1.5f, Tween{}, -1, true);
    }
    {
        auto el = document->GetElementById("animation_event");
        el->Animate("top", Property(Math::RandomReal(250.f), Property::PX), 1.5f, Tween{Tween::Cubic, Tween::InOut});
        el->Animate("left", Property(Math::RandomReal(250.f), Property::PX), 1.5f, Tween{Tween::Cubic, Tween::InOut});
    }

    document->Show( );

}

context::context( )
{
    SetRenderInterface(&instance_of<render_interface>);
    SetSystemInterface(&instance_of<system_interface>);

    Initialise( );

    //------

    const auto [width, height] = std::pair<int, int>(tools::window_size( ));
    ctx_ = CreateContext("main", {width, height});

    //------

#ifdef _DEBUG
    Debugger::Initialise(ctx_);
    Debugger::SetVisible(true);
#endif

    // Tell RmlUi to load the given fonts.
    LoadFontFace(RML_SAMPLE("assets", "LatoLatin-Regular.ttf"));
    // Fonts can be registered as fallback fonts, as in this case to display emojis.
    LoadFontFace(RML_SAMPLE("assets", "NotoEmoji-Regular.ttf"), true);

    LoadFontFace("C:/Windows/fonts/arial.ttf", true);

    ctx_->LoadDocument(RML_SAMPLE("assets", "demo.rml"))->Show( );
    ctx_->LoadDocument(RML_SAMPLE("assets", "window.rml"))->Show( );

    //_Rml_demo_animation(ctx_);

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

context::~context( )
{
    //RemoveContext(ctx_->GetName( ));
    Shutdown( );
}

Context* context::operator->( ) const noexcept
{
    return ctx_;
}

context::operator Context* () const noexcept
{
    return ctx_;
}