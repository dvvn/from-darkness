module;

#include <RmlUi/Core/Context.h>

export module cheat.gui.context;

using namespace Rml;

export namespace cheat::gui
{
    struct context : NonCopyMoveable
    {
        context( );
        ~context( );

        Context* operator->( ) const noexcept;
        operator Context* ()const noexcept;

    private:
        Context* ctx_;
    };
}