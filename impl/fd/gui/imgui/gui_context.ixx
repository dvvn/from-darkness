module;

export module fd.gui.context;

void init_context();
void destroy_context();

export namespace fd::gui
{
    using ::destroy_context;
    using ::init_context;
} // namespace fd::gui
