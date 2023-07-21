#include "sample.h"
#include "variable.h"

#include <imgui.h>

namespace fd
{
struct vars_sample_inner2  : variables_group
{
    variable<bool> vbool{"Test inner", true};

    void on_gui() override
    {
        ImGui::Checkbox(vbool.name(), &vbool);
    }

    string_view name() const override
    {
        return "Inner";
    }
};

struct vars_sample_inner  : variables_group
{
    variable<bool> vbool{"Test inner", true};

    void on_gui() override
    {
        ImGui::Checkbox(vbool.name(), &vbool);
    }

    string_view name() const override
    {
        return "Inner";
    }

    vars_sample_inner2 inner_;

    basic_variables_group *inner() override
    {
        return &inner_;
    }
};

struct vars_sample_next final : variables_group
{
    variable<bool> vbool{"Test next", true};

    void on_gui() override
    {
        ImGui::Checkbox(vbool.name(), &vbool);
    }

    string_view name() const override
    {
        return "Next";
    }

    vars_sample_inner inner_;

    basic_variables_group *inner() override
    {
        return &inner_;
    }
};

struct vars_sample final : variables_group
{
    variable<bool> vbool{"Test", true};

    void on_gui() override
    {
        ImGui::Checkbox(vbool.name(), &vbool);
    }

    string_view name() const override
    {
        return "Sample";
    }

    vars_sample_next next_;
    vars_sample_inner inner_;

    basic_variables_group *next() override
    {
        return &next_;
    }

    basic_variables_group *inner() override
    {
        return 0;
    }
};

FD_OBJECT_IMPL(vars_sample);
}