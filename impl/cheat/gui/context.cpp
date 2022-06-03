﻿module;

#include <cheat/core/object.h>

#include <nstd/core_utils.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/SystemInterface.h>
#ifdef _DEBUG
#include <RmlUi/Debugger.h>
#endif

#include <shlobj_core.h>

#include <memory>
#include <span>

module cheat.gui.context;
import cheat.application_info;

// clang-format off
#define RML_SAMPLES            NSTD_CONCAT(RMLUI_DIR, /Samples/)
#define RML_SAMPLE(_DIR_, _S_) NSTD_STRINGIZE_RAW(RML_SAMPLES)_DIR_"/"_S_
// clang-format on

#if 0
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

    document->Show();
}
#endif

class gui_context
{
    Rml::Context* ctx_;

  public:
    ~gui_context();
    gui_context();

    Rml::Context* operator&() const;
};

CHEAT_OBJECT_BIND(Rml::Context*, context, gui_context);

gui_context::~gui_context()
{
    Rml::Shutdown();
}

static auto _Create_context()
{
    const auto size = cheat::app_info->window.size.client();
    return Rml::CreateContext("main", {size.width(), size.height()});
}

#undef small

class font_path_data
{
    bool small_;

    union
    {
        char buff_[std::size("C:\\Windows\\Fonts") - 1];
        std::vector<char> buff1_;
    };

  public:
    ~font_path_data()
    {
        if (small_)
            return;
        std::destroy_at(&buff_);
    }

    font_path_data(const char* path)
    {
        const auto size = std::char_traits<char>::length(path);

        small_ = size <= std::size(buff_);
        if (small_)
            std::copy(path, path + size, buff_);
        else
            buff1_.assign(path, path + size);
    }

    const char* begin() const
    {
        return small_ ? buff_ : buff1_.data();
    }

    const char* end() const
    {
        return small_ ? (buff_ + std::size(buff_)) : (buff1_.data() + buff1_.size());
    }
};

static auto _Get_system_font(const std::string_view name)
{
    static const auto font_path = []() -> font_path_data {
        char tmp[MAX_PATH];
        if (!SHGetSpecialFolderPathA(NULL, tmp, CSIDL_FONTS, FALSE))
            tmp[0] = '\0';
        return tmp;
    }();
    const std::span font_path_view = font_path;
    std::string out;
    out.reserve(font_path_view.size() + 1 + name.size());
    out.append(font_path_view.begin(), font_path_view.end());
    out += '\\';
    out.append(name);
    return out;
}

gui_context::gui_context()
{
    Rml::SetRenderInterface(&CHEAT_OBJECT_GET(Rml::RenderInterface));
    Rml::SetSystemInterface(&CHEAT_OBJECT_GET(Rml::SystemInterface));

    Rml::Initialise();

    //------

    ctx_ = _Create_context();

    //------
#ifdef _DEBUG
    Rml::Debugger::Initialise(ctx_);
    Rml::Debugger::SetVisible(true);
#endif

    // Tell RmlUi to load the given fonts.
    Rml::LoadFontFace(RML_SAMPLE("assets", "LatoLatin-Regular.ttf"));
    // Fonts can be registered as fallback fonts, as in this case to display emojis.
    Rml::LoadFontFace(RML_SAMPLE("assets", "NotoEmoji-Regular.ttf"), true);

    Rml::LoadFontFace(_Get_system_font("arial.ttf"), true);

    ctx_->LoadDocument(RML_SAMPLE("assets", "demo.rml"))->Show();
    ctx_->LoadDocument(RML_SAMPLE("assets", "window.rml"))->Show();

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

Rml::Context* gui_context::operator&() const
{
    return ctx_;
}
