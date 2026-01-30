/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common component functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <array>

#include <wx/artprov.h>

#include "gen_common.h"

#include "file_codewriter.h"  // FileCodeWriter -- Class to write code to disk
#include "gen_results.h"      // Code generation file writing functions
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "lambdas.h"          // Functions for formatting and storage of lambda events
#include "mainapp.h"          // wxGetApp()
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // WriteCode -- Write code to Scintilla or file

#include "wxue_namespace/wxue.h"  // wxue namespace functions

#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector
#include "wxue_namespace/wxue_view_vector.h"    // wxue::ViewVector

#include "gen_cpp.h"     // CppCodeGenerator -- Generate C++ code
#include "gen_perl.h"    // PerlCodeGenerator class
#include "gen_python.h"  // PythonCodeGenerator -- Generate wxPython code
#include "gen_ruby.h"    // RubyCodeGenerator -- Generate wxRuby code
#include "gen_xrc.h"     // XrcGenerator -- Generate XRC code

void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr)
{
    if (node->is_PropValue(prop_class_access, "none"))
    {
        set_src.insert(include);
    }
    else
    {
        set_hdr.insert(include);
    }
}

void ColourCode(Code& code, GenEnum::PropName prop_name)
{
    if (!code.HasValue(prop_name))
    {
        code.Add("wxNullColour");
    }
    else
    {
        auto colour = code.node()->as_wxColour(prop_name);
        code.Object("wxColour").QuotedString(colour) += ')';
    }
}

auto GenerateQuotedString(const wxue::string& str) -> wxue::string
{
    wxue::string code;

    if (str.size())
    {
        auto str_with_escapes = ConvertToCodeString(str);

        bool has_utf_char = false;
        for (auto iter: str_with_escapes)
        {
            if (iter < 0)
            {
                has_utf_char = true;
                break;
            }
        }

        if (has_utf_char)
        {
            // While this may not be necessary for non-Windows systems, it does ensure the code
            // compiles on all platforms.
            if (Project.as_bool(prop_internationalize))
            {
                code << "_(wxString::FromUTF8(\"" << str_with_escapes << "\"))";
            }
            else
            {
                code << "wxString::FromUTF8(\"" << str_with_escapes << "\")";
            }
        }
        else
        {
            if (Project.as_bool(prop_internationalize))
            {
                code << "_(\"" << str_with_escapes << "\")";
            }
            else
            {
                code << "\"" << str_with_escapes << "\"";
            }
        }
    }
    else
    {
        code << "wxEmptyString";
    }

    return code;
}

auto GenerateQuotedString(Node* node, GenEnum::PropName prop_name) -> wxue::string
{
    if (node->HasValue(prop_name))
    {
        return GenerateQuotedString(node->as_string(prop_name));
    }

    return wxue::string("wxEmptyString");
}

// clang-format off


namespace
{
    // List of valid component parent types
    constexpr std::array<GenType, 11> s_GenParentTypes = {
        type_auinotebook,
        type_bookpage,
        type_choicebook,
        type_container,
        type_listbook,
        type_notebook,
        type_panel,
        type_ribbonpanel,
        type_simplebook,
        type_splitter,
        type_wizardpagesimple,
    };
} // anonymous namespace

// clang-format on

auto get_ParentName(Node* node, GenLang language) -> wxue::string
{
    auto* parent = node->get_Parent();
    while (parent)
    {
        if (parent->is_Sizer())
        {
            if (parent->is_StaticBoxSizer())
            {
                return (wxue::string() << parent->get_NodeName(language) << "->GetStaticBox()");
            }
        }
        if (parent->is_Form())
        {
            return wxue::string("this");
        }

        for (const auto iter: s_GenParentTypes)
        {
            if (parent->is_Type(iter))
            {
                wxue::string name = parent->get_NodeName(language);
                if (parent->is_Gen(gen_wxCollapsiblePane))
                {
                    name << "->GetPane()";
                }
                return name;
            }
        }
        parent = parent->get_Parent();
    }

    ASSERT_MSG(parent, wxue::string() << node->get_NodeName() << " has no parent!");
    return { "internal error" };
}

static void GenStyle(Node* node, wxue::string& code, const char* prefix)
{
    wxue::string all_styles;

    if (node->HasValue(prop_tab_position) &&
        !node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        if (all_styles.size())
        {
            all_styles << '|';
        }
        all_styles << node->as_string(prop_tab_position);
    }

    if (node->HasValue(prop_orientation) &&
        !node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        if (all_styles.size())
        {
            all_styles << '|';
        }
        all_styles << node->as_string(prop_orientation);
    }

    if (node->is_Gen(gen_wxRichTextCtrl))
    {
        if (all_styles.size())
        {
            all_styles << '|';
        }
        all_styles << "wxRE_MULTILINE";
    }

    if (node->HasValue(prop_style))
    {
        if (all_styles.size())
        {
            all_styles << '|';
        }
        if (prefix)
        {
            all_styles << node->as_constant(prop_style, prefix);
        }
        else
        {
            all_styles << node->as_string(prop_style);
        }
    }

    if (node->HasValue(prop_window_style))
    {
        if (all_styles.size())
        {
            all_styles << '|';
        }
        all_styles << node->as_string(prop_window_style);
    }

    if (node->is_Gen(gen_wxListView))
    {
        if (all_styles.size())
        {
            all_styles << '|';
        }
        all_styles << node->as_string(prop_mode);
    }

    if (all_styles.empty())
    {
        code << "0";
    }
    else
    {
        code << all_styles;
    }
}

auto GetStyleInt(Node* node, const char* prefix) -> int
{
    wxue::string styles;

    // If prefix is non-null, this will convert friendly names to wxWidgets constants
    GenStyle(node, styles, prefix);

    int result = 0;
    // Can't use multiview because get_ConstantAsInt() searches an unordered_map which requires a
    // std::string to pass to it
    wxue::StringVector mstr(styles, '|');
    for (const auto& iter: mstr)
    {
        // Friendly names will have already been converted, so normal lookup works fine.
        result |= NodeCreation.get_ConstantAsInt(iter.ToStdString());
    }
    return result;
}

auto GetBitlistInt(Node* node, GenEnum::PropName prop_name) -> int
{
    int result = 0;
    // Can't use multiview because get_ConstantAsInt() searches an unordered_map which requires a
    // std::string to pass to it
    wxue::StringVector mstr(std::string_view(node->as_string(prop_name)), '|');
    for (const auto& iter: mstr)
    {
        // Friendly names will have already been converted, so normal lookup works fine.
        result |= NodeCreation.get_ConstantAsInt(iter.ToStdString());
    }
    return result;
}

struct BTN_BMP_TYPES
{
    GenEnum::PropName prop_name;
    const char* function_name;
};

inline constexpr auto btn_bmp_types = std::to_array<BTN_BMP_TYPES>({
    { .prop_name = prop_bitmap, .function_name = "SetBitmap" },
    { .prop_name = prop_disabled_bmp, .function_name = "SetBitmapDisabled" },
    { .prop_name = prop_pressed_bmp, .function_name = "SetBitmapPressed" },
    { .prop_name = prop_focus_bmp, .function_name = "SetBitmapFocus" },
    { .prop_name = prop_current, .function_name = "SetBitmapCurrent" },
});

auto GenBtnBitmapCode(Node* node, wxue::string& code, bool is_single) -> bool
{
    bool has_additional_bitmaps =
        (node->HasValue(prop_disabled_bmp) || node->HasValue(prop_pressed_bmp) ||
         node->HasValue(prop_focus_bmp) || node->HasValue(prop_current));
    if (code.size())
    {
        code << '\n';
    }

    if (has_additional_bitmaps)
    {
        if (code.size() && !(code.back() == '\n'))
        {
            code << '\n';
        }
        code << "{\n";
    }

    wxue::string bundle_code;
    bool is_vector_generated = false;

    for (const auto& iter: btn_bmp_types)
    {
        if (node->HasValue(iter.prop_name))
        {
            bundle_code.clear();
            bool is_code_block = GenerateBundleCode(node->as_string(iter.prop_name), bundle_code);
            if (is_code_block)
            {
                if (is_vector_generated)
                {
                    code << "\n\n\tbitmaps.clear();";
                    // find end of wxVector<wxBitmap> bitmaps;
                    bundle_code.erase(0, bundle_code.find(';'));
                }
                else
                {
                    is_vector_generated = true;
                }

                // GenerateBundleCode assumes an indent within an indent
                bundle_code.Replace("\t\t\t", "\t", true);
                // if has_additional_bitmaps is true, we already have an opening brace
                code << (has_additional_bitmaps && bundle_code.size() > 0 ? bundle_code.substr(1) :
                                                                            bundle_code);
                code << "\t" << node->get_NodeName() << "->" << iter.function_name;
                code << "(wxBitmapBundle::FromBitmaps(bitmaps));";

                if (!has_additional_bitmaps)
                {
                    code << "\n}";
                }
            }
            else
            {
                if (code.size() && !(code.back() == '\n'))
                {
                    code << '\n';
                }
                code << "\t" << node->get_NodeName() << "->" << iter.function_name << "("
                     << bundle_code << ");";
            }
        }
        if (is_single)
        {
            // Means the caller only wants prop_bitmap
            break;
        }
    }

    if (has_additional_bitmaps)
    {
        code << "\n}";
    }

    return is_vector_generated;
}

auto GenerateBitmapCode(const wxue::string& description) -> wxue::string
{
    wxue::string code;

    if (description.empty())
    {
        code << "wxNullBitmap";
        return code;
    }

    wxue::ViewVector parts(description, BMP_PROP_SEPARATOR, wxue::TRIM::both);

    if (parts[IndexType].starts_with("SVG"))
    {
        code << "wxNullBitmap /* SVG images require wxWidgets 3.1.6 */";
        return code;
    }
    if (parts[IndexImage].empty())
    {
        code << "wxNullBitmap";
        return code;
    }

    if (parts[IndexType].contains("Art"))
    {
        wxue::string art_id(parts[IndexArtID]);
        wxue::string art_client;
        if (auto pos = art_id.find('|'); wxue::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "wxArtProvider::GetBitmap(" << art_id;
        if (art_client.size())
        {
            code << ", " << art_client;
        }
        code << ')';

        return code;
    }

    wxue::string result;
    if (parts[IndexType] == "XPM")
    {
        code << "wxImage(";

        wxue::string name(wxue::find_filename(parts[IndexImage]));
        name.remove_extension();
        code << name << "_xpm)";
    }
    else
    {
        code << "wxueImage(";

        wxue::string name(wxue::find_filename(parts[1]));
        name.remove_extension();
        name.Replace(".", "_", true);  // wxFormBuilder writes files with the extra dots that have
                                       // to be converted to '_'

        if (parts[IndexType].starts_with("Embed"))
        {
            auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                name = "wxue_img::" + embed->base_image().array_name;
            }
        }

        code << name << ", sizeof(" << name << "))";
    }
    return code;
}

bool GenerateBundleCode(const wxue::string& description, wxue::string& code)
{
    if (description.empty())
    {
        code << "wxNullBitmap";
        return false;
    }

    wxue::ViewVector parts(description, BMP_PROP_SEPARATOR, wxue::TRIM::both);

    if (parts[IndexImage].empty())
    {
        code << "wxNullBitmap";
        return false;
    }

    if (parts[IndexType].contains("Art"))
    {
        wxue::string art_id(parts[IndexArtID]);
        wxue::string art_client;
        if (auto pos = art_id.find('|'); wxue::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "wxArtProvider::GetBitmapBundle(" << art_id << ", ";

        // Note that current documentation states that the client is required, but the header file
        // says otherwise
        if (art_client.size())
        {
            code << art_client;
        }

        if (parts.size() > IndexSize && parts[IndexSize].size())
        {
            wxSize svg_size { -1, -1 };
            svg_size = GetSizeInfo(parts[IndexSize]);

            if (svg_size != wxDefaultSize)
            {
                code << ", wxSize(" << svg_size.x << ", " << svg_size.y << ')';
            }
        }
        code << ')';
    }

    else if (parts[IndexType] == "XPM")
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            // We get here if there is an Image form that contains the function to retrieve this
            // bundle.
            code << function_name;
            return false;
        }

        if (const auto* bundle = ProjectImages.GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                code << "wxBitmapBundle::FromBitmap(wxImage(";
                wxue::string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                code << name << "_xpm))";
            }
            else if (bundle->lst_filenames.size() == 2)
            {
                code << "wxBitmapBundle::FromBitmaps(wxImage(";
                wxue::string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                code << name << "_xpm), ";
                name = bundle->lst_filenames[1].filename();
                name.remove_extension();
                code << "wxImage(" << name << "_xpm))";
            }
            else
            {
                code << "{\n\t\t\twxVector<wxBitmap> bitmaps;\n";
                for (const auto& iter: bundle->lst_filenames)
                {
                    wxue::string name(iter.filename());
                    name.remove_extension();
                    code << "\t\t\tbitmaps.push_back(wxImage(" << name << "_xpm));\n";
                }

                // Return true to indicate a code block was generated
                return true;
            }
        }
        else
        {
            FAIL_MSG(wxue::string(description) << " was not converted to a bundle ahead of time!")

            // This should never happen, but if it does, at least generate something that will
            // compiler
            code << "wxImage(";
            wxue::string name(wxue::find_filename(parts[IndexImage]));
            name.remove_extension();
            code << name << "_xpm)";
        }
    }
    else if (description.starts_with("SVG"))
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            // We get here if there is an Image form that contains the function to retrieve this
            // bundle.
            code << function_name;
            return false;
        }

        auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (!embed)
        {
            MSG_WARNING(wxue::string() << description << " not embedded!");
            code << "wxNullBitmap";
            return false;
        }

        wxSize svg_size { -1, -1 };
        if (parts[IndexSize].size())
        {
            svg_size = GetSizeInfo(parts[IndexSize]);
        }

        wxue::string name = "wxue_img::" + embed->base_image().array_name;
        code << "wxueBundleSVG(" << name << ", "
             << (to_size_t) (embed->base_image().array_size & 0xFFFFFFFF) << ", ";
        code << (to_size_t) (embed->base_image().array_size >> 32) << ", wxSize(" << svg_size.x
             << ", " << svg_size.y << "))";
    }
    else
    {
        // Code generation differs for 1, 2 or 3+ images, and each of those differ depending on
        // if there is an Images List function to load the image, and it further differs
        // depending on if the Project specifies wxWidgets 3.1 as the minimum.

        ASSERT_MSG(description.starts_with("Embed"), "Unknown image type!");
        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            // We get here if there is an Images List that contains the function to retrieve this
            // bundle.
            code << function_name;
            return false;
        }

        if (const auto* bundle = ProjectImages.GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                wxue::string name(bundle->lst_filenames[0].filename());

                if (auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                {
                    if (auto function_name = ProjectImages.GetBundleFuncName(embed);
                        function_name.size())
                    {
                        code << function_name;
                        return false;
                    }

                    name = "wxue_img::" + embed->base_image().array_name;
                }

                code << "wxBitmapBundle::FromBitmap(wxueImage(";
                code << name << ", sizeof(" << name << ")))";
            }
            else if (bundle->lst_filenames.size() == 2)
            {
                wxue::string first_name;
                wxue::string second_name;
                wxue::string first_function;
                wxue::string second_function;

                if (auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                {
                    first_function = ProjectImages.GetBundleFuncName(embed);
                    first_name = "wxue_img::" + embed->base_image().array_name;
                }
                if (auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed)
                {
                    second_function = ProjectImages.GetBundleFuncName(embed);
                    second_name = "wxue_img::" + embed->base_image().array_name;
                }
                code << "wxBitmapBundle::FromBitmaps(\n\t\t";
                if (first_function.size())
                {
                    code << first_function;
                }
                else
                {
                    code << "wxueImage(";
                    code << first_name << ", sizeof(" << first_name
                         << "))";  // one less closing parenthesis
                }
                code << ",\n\t\t";

                if (second_function.size())
                {
                    code << second_function;
                }
                else
                {
                    code << "wxueImage(";
                    code << second_name << ", sizeof(" << second_name << "))";
                }
                code << ")";  // Close FromBitmaps()
            }
            else if (bundle->lst_filenames.size() > 2)
            {
                wxue::string name;
                wxue::string function;

                code << "{\n\twxVector<wxBitmap> bitmaps;\n";
                for (const auto& iter: bundle->lst_filenames)
                {
                    if (auto* embed = ProjectImages.GetEmbeddedImage(iter); embed)
                    {
                        function = ProjectImages.GetBundleFuncName(embed);
                        name = "wxue_img::" + embed->base_image().array_name;
                    }
                    code << "\tbitmaps.push_back(";
                    if (function.size())
                    {
                        code << function << ");\n";
                    }
                    else
                    {
                        code << "wxueImage(";
                        code << name << ", sizeof(" << name << ")));\n";
                    }
                }

                // Return true to indicate a code block was generated
                return true;
            }
            else
            {
                FAIL_MSG(wxue::string(description)
                         << " was not converted to a bundle ahead of time!")

                // This should never happen, but if it does, at least generate something that will
                // compile
                code << "wxNullBitmap";
            }
        }
        else
        {
            FAIL_MSG(wxue::string(description) << " was not converted to a bundle ahead of time!")

            // This should never happen, but if it does, at least generate something that will
            // compiler
            code << "wxNullBitmap";
        }
    }

    return false;
}

void GenFormSettings(Code& code)
{
    const auto* node = code.node();
    if (!node->is_Gen(gen_PanelForm) && !node->is_Gen(gen_wxToolBar))
    {
        const auto max_size = node->as_wxSize(prop_maximum_size);
        const auto min_size = node->as_wxSize(prop_minimum_size);
        if (min_size != wxDefaultSize || max_size != wxDefaultSize)
        {
            code.Eol(eol_if_needed).FormFunction("SetSizeHints(");
            if (min_size.GetX() != -1 || min_size.GetY() != -1)
            {
                code.WxSize(prop_minimum_size);
            }
            else
            {
                code.Add("wxDefaultSize");
            }

            if (max_size.GetX() != -1 || max_size.GetY() != -1)
            {
                code.WxSize(prop_maximum_size);
            }

            code.EndFunction();
        }
    }

    if (code.HasValue(prop_window_extra_style))
    {
        code.Eol(eol_if_needed).FormFunction("SetExtraStyle(").FormFunction("GetExtraStyle() | ");
        code.Add(prop_window_extra_style).EndFunction();
    }

    code.GenFontColourSettings();
}

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
// Used when generating code.
auto ConvertToCodeString(const wxue::string& text) -> wxue::string
{
    wxue::string result;

    for (auto c: text)
    {
        switch (c)
        {
            case '"':
                result += "\\\"";
                break;

            // This generally isn't needed for C++, but is needed for Python
            case '\'':
                result += "\\'";
                break;

            case '\\':
                result += "\\\\";
                break;

            case '\t':
                result += "\\t";
                break;

            case '\n':
                result += "\\n";
                break;

            case '\r':
                result += "\\r";
                break;

            default:
                result += c;
                break;
        }
    }
    return result;
}

auto GenGetSetCode(Node* node) -> std::optional<wxue::string>
{
    const auto& get_name = node->as_string(prop_get_function);
    const auto& set_name = node->as_string(prop_set_function);
    if (get_name.empty() && set_name.empty())
    {
        return {};
    }

    if (const auto& var_name = node->as_string(prop_validator_variable); var_name.size())
    {
        auto val_data_type = node->get_ValidatorDataType();
        if (val_data_type.empty())
        {
            return {};
        }
        wxue::string code;
        if (val_data_type == "wxString" || val_data_type == "wxFileName" ||
            val_data_type == "wxArrayInt")
        {
            if (get_name.size())
            {
                code << "\tconst " << val_data_type << "& " << get_name << "() const { return "
                     << var_name << "; }";
            }
            if (set_name.size())
            {
                if (code.size())
                {
                    code << "\n";
                }
                code << "\tvoid " << set_name << "(const " << val_data_type << "& value) { "
                     << var_name << " = value; }";
            }
            if (code.empty())
            {
                return {};
            }
            return code;
        }

        if (get_name.size())
        {
            code << '\t' << val_data_type << ' ' << get_name << "() const { return " << var_name
                 << "; }";
        }
        if (set_name.size())
        {
            if (code.size())
            {
                code << "\n";
            }
            code << "\tvoid " << set_name << "(" << val_data_type << " value) { " << var_name
                 << " = value; }";
        }
        if (code.empty())
        {
            return {};
        }
        return code;
    }

    return {};
}

void GenValidatorSettings(Code& code)
{
    // assignment just for convenience
    const auto* node = code.node();

    // Make a copy of this -- most languages will need to modify it.
    auto var_name = node->as_string(prop_validator_variable);
    // Unless there is a variable name, we ignore the entire validator section
    if (var_name.empty())
    {
        return;
    }

    // Python supports user-created validator classes, but not any of the wxWidgets validators.
    // From Python docs: "neither TextValidator nor GenericValidator are implemented in wxPython"
    if (code.is_python())
    {
        code.Eol(eol_if_needed);
        code.Str("# wxPython only supports custom validators, not the wxWidgets validators.");
        return;
    }

    code.Eol(eol_if_needed);
    if (code.is_ruby())
    {
        if (node->is_Gen(gen_StaticCheckboxBoxSizer))
        {
            var_name = "@" + node->as_string(prop_checkbox_var_name);
        }
        else if (node->is_Gen(gen_StaticRadioBtnBoxSizer))
        {
            var_name = "@" + node->as_string(prop_radiobtn_var_name);
        }
        else
        {
            var_name.insert(0, "@");
        }
        code.Str("# to retrieve: ").Str(var_name).Str(" = ");
        code.NodeName().Str(".get_validator.get_value()").Eol();
        code.Str(var_name).Str(" = nil").Eol();
        code.NodeName();
    }

    if (code.is_cpp())
    {
        if (node->is_Gen(gen_StaticCheckboxBoxSizer))
        {
            code.Add(prop_checkbox_var_name);
        }
        else if (node->is_Gen(gen_StaticRadioBtnBoxSizer))
        {
            code.Add(prop_radiobtn_var_name);
        }
        else
        {
            code.NodeName();
        }
    }

    if (node->is_Gen(gen_wxRearrangeCtrl))
    {
        code.Function("GetList()");
    }

    code.Function("SetValidator(");

    if (!node->HasProp(prop_validator_data_type))
    {
        // Used for things like checkboxes, radio buttons, etc.
        if (code.is_cpp())
        {
            code.Add("wxGenericValidator(&").Str(var_name).Str(")").EndFunction();
        }
        else
        {
            code.Add("wxGenericValidator").Str(".new").EndFunction();
        }
        return;
    }

    const auto& data_type = node->as_string(prop_validator_data_type);
    ASSERT(data_type.size())
    if (data_type.empty())
    {  // theoretically impossible
        FAIL_MSG(wxString() << "No validator data type for " << wxString(node->get_NodeName()));
        code.Add("wxDefaultValidator").EndFunction();
        return;
    }
    auto style = node->as_string(prop_validator_style);
    style.Replace("wxFILTER_NONE", "", true);
    if (style.starts_with('|'))
    {
        style.erase(0, 1);
    }

    auto validator_type = node->get_ValidatorType();

    if (validator_type == "wxGenericValidator")
    {
        code.Add(validator_type);
        if (code.is_ruby())
        {
            code.Str(".new");
        }

        // wxRuby doesn't add the var_name
        if (code.is_cpp())
        {
            code.Str("(&").Str(var_name);
        }
        code.Str(")").EndFunction();
        return;
    }

    if (validator_type == "wxIntegerValidator")
    {
        if (code.is_cpp())
        {
            code.Add(validator_type).Str("<").Str(data_type).Str(">(");
        }
        else if (code.is_ruby())
        {
            if (data_type.contains("unsigned"))
            {
                code.Str("Wx::UnsignedValidator.new");
                if (style.size())
                {
                    code << '(';
                }
            }
            else
            {
                code.Add(validator_type).Str(".new");
                if (style.size())
                {
                    code << '(';
                }
            }
        }
    }
    else
    {
        if (code.is_ruby() && validator_type == "wxFloatingPointValidator")
        {
            code.Str("Wx::FloatValidator").AddIfRuby(".new") << '(';
        }
        else
        {
            code.Add(validator_type).AddIfRuby(".new") << '(';
        }
    }

    wxue::StringVector styles(style, '|', wxue::TRIM::both);
    if (validator_type == "wxTextValidator")
    {
        if (style.contains("wxFILTER_"))
        {
            wxue::string filters;
            for (const auto& iter: styles)
            {
                if (iter.starts_with("wxFILTER_"))
                {
                    if (filters.size())
                    {
                        filters << '|';
                    }
                    filters << iter.ToStdString();
                }
            }
            code.Add(filters);
        }
        else
        {
            code.Add("wxFILTER_NONE");
        }

        // wxRuby doesn't add the var_name
        if (code.is_cpp())
        {
            code.Comma().Str("&").Str(var_name);
        }
        if (code.is_ruby() && code.back() == '(')
        {
            code.pop_back();
        }
        else
        {
            code << ')';
        }
        code.EndFunction();
    }
    else
    {
        if (validator_type == "wxFloatingPointValidator")
        {
            if (node->as_int(prop_precision) > 0)
            {
                code.Add(prop_precision).Comma();
            }
        }

        // wxRuby doesn't add the var_name
        if (code.is_cpp())
        {
            code.Str("&").Str(var_name);
        }

        if (style.contains("wxNUM_"))
        {
            wxue::string num_styles;
            for (const auto& iter: styles)
            {
                if (iter.starts_with("wxNUM_"))
                {
                    if (num_styles.size())
                    {
                        num_styles << '|';
                    }
                    num_styles << iter.ToStdString();
                }
            }
            if (code.is_cpp())
            {
                code.Comma();
            }
            code.Add(num_styles);
        }
        if (code.is_ruby() && code.back() == '(')
        {
            code.pop_back();
        }
        else
        {
            code << ')';
        }
        code.EndFunction();
    }
    if (validator_type == "wxIntegerValidator" || validator_type == "wxFloatingPointValidator")
    {
        if (node->HasValue(prop_minValue))
        {
            if (code.is_cpp())
            {
                code.Eol()
                    .Str("wxStaticCast(")
                    .NodeName()
                    .Str("->GetValidator(), ")
                    .Str(validator_type);
                if (validator_type == "wxIntegerValidator")
                {
                    code.Str("<").Str(data_type).Str(">");
                }
                code.Str(")->SetMin(").Add(prop_minValue).Str(");");
            }
            else if (code.is_ruby())
            {
                code.Eol().NodeName().Str(".get_validator.set_min(").Add(prop_minValue).Str(")");
            }
        }
        if (node->HasValue(prop_maxValue))
        {
            if (code.is_cpp())
            {
                code.Eol()
                    .Str("wxStaticCast(")
                    .NodeName()
                    .Str("->GetValidator(), ")
                    .Str(validator_type);
                if (validator_type == "wxIntegerValidator")
                {
                    code.Str("<").Str(data_type).Str(">");
                }
                code.Str(")->SetMax(").Add(prop_maxValue).Str(");");
            }
            else if (code.is_ruby())
            {
                code.Eol().NodeName().Str(".get_validator.set_max(").Add(prop_maxValue).Str(")");
            }
        }
    }
}

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate everything
// needed to set the window's icon.

auto GenerateIconCode(const wxue::string& description) -> wxue::string
{
    wxue::string code;

    if (description.empty())
    {
        return code;
    }

    wxue::ViewVector parts(description, BMP_PROP_SEPARATOR, wxue::TRIM::both);

    if (parts.size() < 2 || parts[IndexImage].empty())
    {
        return code;
    }

    if (parts[IndexType] == "XPM")
    {
        // In theory, we could create an alpha channel using black as the transparency, but it just
        // doesn't make sense for the user to be using XPM files as an icon.
        code << "// XPM files do not contain an alpha channel and cannot be used as an icon.";
        return code;
    }

    if (parts[IndexType].contains("Art"))
    {
        wxue::string art_id(parts[IndexArtID]);
        wxue::string art_client;
        if (auto pos = art_id.find('|'); wxue::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "SetIcon(wxArtProvider::GetBitmapBundle(" << art_id << ", ";
        // Note that current documentation states that the client is required, but the header file
        // says otherwise
        if (art_client.size())
        {
            code << art_client;
        }
        code << ").GetIconFor(this));\n";
    }
    else if (description.starts_with("SVG"))
    {
        auto* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (!embed)
        {
            FAIL_MSG(wxue::string() << description << " not embedded!")
            return code;
        }

        auto svg_size = GetSizeInfo(parts[IndexSize]);

        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            code << "SetIcon(" << function_name;
        }
        else
        {
            wxue::string name = "wxue_img::" + embed->base_image().array_name;
            code << "SetIcon(wxueBundleSVG(" << name << ", "
                 << (to_size_t) (embed->base_image().array_size & 0xFFFFFFFF) << ", ";
            code << (to_size_t) (embed->base_image().array_size >> 32) << ", wxSize(" << svg_size.x
                 << ", " << svg_size.y << "))";
        }

        code << ".GetIconFor(this));\n";
        return code;
    }
    else
    {
        if (const auto* bundle = ProjectImages.GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                code << "SetIcon(wxBitmapBundle::FromBitmap(wxueImage(";
                wxue::string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                name.Replace(".", "_", true);  // fix wxFormBuilder header files

                if (parts[IndexType].starts_with("Embed"))
                {
                    auto* embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                    if (embed)
                    {
                        name = "wxue_img::" + embed->base_image().array_name;
                    }
                }

                code << name << ", sizeof(" << name << ")))";
                code << ".GetIconFor(this));\n";
            }
            else
            {
                code << "{\n\twxIconBundle icon_bundle;\n\twxIcon icon;\n";
                for (const auto& iter: bundle->lst_filenames)
                {
                    wxue::string name(iter.filename());
                    name.remove_extension();
                    name.Replace(".", "_", true);  // fix wxFormBuilder header files
                    if (parts[IndexType].starts_with("Embed"))
                    {
                        auto* embed = ProjectImages.GetEmbeddedImage(iter);
                        if (embed)
                        {
                            name = "wxue_img::" + embed->base_image().array_name;
                        }
                    }
                    code << "\ticon.CopyFromBitmap(wxueImage(" << name << ", sizeof(" << name
                         << ")));\n";
                    code << "\ticon_bundle.AddIcon(icon);\n";
                }
                code << "\tSetIcons(icon_bundle);\n}\n";
            }
        }
        else
        {
            FAIL_MSG(wxue::string(description) << " was not converted to a bundle ahead of time!")
            return code;
        }
    }

    return code;
}

// This is called to add a tool to wxToolBar, wxAuiToolBar or wxRibbonToolBar
void GenToolCode(Code& code)
{
    const auto* node = code.node();
    code.Eol(eol_if_needed);
    bool need_variable_result =
        (node->HasValue(prop_var_name) &&
         ((node->as_string(prop_class_access) != "none") || node->is_Gen(gen_tool_dropdown) ||
          (node->is_Gen(gen_auitool) &&
           node->as_string(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")));

    if (node->as_bool(prop_disabled) ||
        (node->as_string(prop_id) == "wxID_ANY" && node->get_InUseEventCount()))
    {
        code.AddAuto().NodeName();
        code += " = ";
        need_variable_result = false;  // make certain we don't add this again
    }

    if (need_variable_result)
    {
        if (node->is_Local())
        {
            code.AddIfCpp("auto* ");
        }
        code.NodeName().Add(" = ");
    }

    if (node->is_Parent(gen_wxToolBar) || node->is_Parent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddTool(").as_string(prop_id).Comma();
    }
    else
    {
        if (code.is_python())
        {
            code += "self.";
        }
        code.Add("AddTool(").as_string(prop_id).Comma();
    }

    code.QuotedString(prop_label).Comma();

    if (!code.HasValue(prop_bitmap))
    {
        code.Add("wxNullBitmap");
    }
    else
    {
        wxue::StringVector parts(node->as_string(prop_bitmap), BMP_PROP_SEPARATOR,
                                 wxue::TRIM::both);

        if (parts.size() <= 1 || parts[IndexImage].empty())
        {
            code.Add("wxNullBitmap");
        }
        else
        {
            code.GenerateBundleParameter(parts);
        }
    }

    if (!node->HasValue(prop_tooltip) && !node->HasValue(prop_statusbar))
    {
        if (node->is_Gen(gen_tool_dropdown))
        {
            code.Comma().Add("wxEmptyString").Comma().Add("wxITEM_DROPDOWN");
        }
        else if (node->as_string(prop_kind) != "wxITEM_NORMAL")
        {
            code.Comma().Add("wxEmptyString").Comma().as_string(prop_kind);
        }
    }

    else if (node->HasValue(prop_tooltip) && !node->HasValue(prop_statusbar))
    {
        code.Comma().QuotedString(prop_tooltip);
        if (node->is_Gen(gen_tool_dropdown))
        {
            code.Comma().Add("wxITEM_DROPDOWN");
        }
        else if (node->as_string(prop_kind) != "wxITEM_NORMAL")
        {
            code.Comma().as_string(prop_kind);
        }
    }

    else if (node->HasValue(prop_statusbar))
    {
        code.Comma().Add("wxNullBitmap").Comma().as_string(prop_kind).Comma();

        code.QuotedString(prop_tooltip).Comma().QuotedString(prop_statusbar);
        if (node->is_Gen(gen_auitool))
        {
            code.Comma();
            code.AddIfCpp("nullptr");
            code.AddIfPython("None");
            code.AddIfRuby("nil");
        }
    }
    code.EndFunction();
}

auto BitmapList(Code& code, const GenEnum::PropName prop) -> bool
{
    auto* node = code.node();  // for convenience
    // Note that Ruby always uses a function, and therefore has no need for a list
    if (!node->HasValue(prop) || code.is_ruby())
    {
        return false;
    }

    const auto& description = node->as_string(prop);
    wxue::ViewVector parts(description, BMP_PROP_SEPARATOR, wxue::TRIM::both);

    if (parts[IndexImage].empty() || parts[IndexType].contains("Art") ||
        parts[IndexType].contains("SVG"))
    {
        return false;
    }

    const auto* bundle = ProjectImages.GetPropertyImageBundle(description);

    if (!bundle || bundle->lst_filenames.size() < 3)
    {
        return false;
    }

    // If we get here, then the bitmaps need to be put into a vector

    bool is_xpm = (parts[IndexType] == "XPM");

    if (code.is_python())
    {
        auto path = MakePythonPath(code.node());

        code += "bitmaps = [ ";
        bool needs_comma = false;
        for (const auto& iter: bundle->lst_filenames)
        {
            if (needs_comma)
            {
                code.UpdateBreakAt();
                code.Comma(false).Eol().Tab(3);
            }
            wxue::string name(iter);
            name.make_absolute();
            name.make_relative(path);
            name.backslashestoforward();

            code.Str("wxBitmap(").QuotedString(name);
            if (is_xpm)
            {
                code.Comma().Str("wx.BITMAP_TYPE_XPM");
            }
            code += ")";
            needs_comma = true;
        }
        code += " ]\n";
        code.UpdateBreakAt();

        return true;
    }

    //////////////// C++ code starts here ////////////////
    bool use_lambda = (node->HasValue(prop_var_name) &&
                       (node->is_Gen(gen_tool_dropdown) ||
                        (node->is_Gen(gen_auitool) &&
                         node->as_string(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")));

    if (use_lambda)
    {
        code.Str("auto make_").NodeName() << "_bundle = [&]() -> wxBitmapBundle";
    }
    code.OpenBrace().Add("wxVector<wxBitmap> bitmaps;");

    for (const auto& iter: bundle->lst_filenames)
    {
        wxue::string name(iter.filename());
        name.remove_extension();
        if (is_xpm)
        {
            code.Eol().Str("bitmaps.push_back(wxImage(") << name << "_xpm));";
        }
        else
        {
            name.Replace(".", "_", true);  // fix wxFormBuilder header files
            if (parts[IndexType].starts_with("Embed"))
            {
                auto* embed = ProjectImages.GetEmbeddedImage(iter);
                if (embed)
                {
                    name = "wxue_img::" + embed->base_image().array_name;
                }
            }
            code.Eol().Str("bitmaps.push_back(wxueImage(") << name << ", sizeof(" << name << ")));";
        }
    }
    code.Eol();

    if (use_lambda)
    {
        code.Str("return wxBitmapBundle::FromBitmaps(bitmaps);").CloseBrace();
        code.pop_back();  // remove the linefeed
        code.Str(";").Eol();
    }

    // Caller should add the function that uses the bitmaps, add the closing brace, and if
    // prop_wxWidgets_version == 3.1, follow this with a #else and the alternate code.

    return true;
}

void OnGenerateSingleLanguage(GenLang language)
{
    auto* form = wxGetMainFrame()->getSelectedNode();
    if (form && !form->is_Form())
    {
        form = form->get_Form();
    }
    if (!form)
    {
        wxMessageBox("You must select a form before you can generate code.", "Code Generation");
        return;
    }

    GenResults results;
    results.SetNodes(form);
    results.SetLanguages(language);
    results.SetMode(GenResults::Mode::generate_and_write);
    std::ignore = results.Generate();

    wxue::string msg;
    if (results.GetUpdatedFiles().size())
    {
        if (results.GetUpdatedFiles().size() == 1)
        {
            msg << "1 file was updated";
        }
        else
        {
            msg << results.GetUpdatedFiles().size() << " files were updated";
        }
        msg << '\n';
    }
    else
    {
        msg << "Generated file is current";
    }

    if (results.GetMsgs().size())
    {
        for (const auto& iter: results.GetMsgs())
        {
            msg << '\n';
            msg << iter;
        }
    }

    wxMessageBox(msg, wxue::string() << GenLangToString(language) << " Code Generation",
                 wxOK | wxICON_INFORMATION);
}

void OnGenerateLanguage(GenLang language)
{
    GenResults results;
    results.SetNodes(Project.get_ProjectNode());
    results.SetLanguages(language);
    results.SetMode(GenResults::Mode::generate_and_write);
    std::ignore = results.Generate();

    wxue::string msg;
    if (results.GetUpdatedFiles().size())
    {
        if (results.GetUpdatedFiles().size() == 1)
        {
            msg << "1 file was updated";
        }
        else
        {
            msg << " files were updated";
        }
        msg << '\n';
    }
    else
    {
        msg << "All " << results.GetFileCount() << " generated files are current";
    }

    if (results.GetMsgs().size())
    {
        for (const auto& iter: results.GetMsgs())
        {
            msg << '\n';
            msg << iter;
        }
    }

    wxMessageBox(msg, wxue::string() << GenLangToString(language) << " Code Generation",
                 wxOK | wxICON_INFORMATION);
}

wxue::string GatherPerlNodeEvents(Node* node)
{
    std::set<std::string_view> event_set;

    auto append_perl_events = [&](Node* node)
    {
        for (const auto& iter: node->get_MapEvents())
        {
            // Only add the event if a handler was specified
            if (iter.second.get_value().size())
            {
                std::string_view event_name = iter.first;
                if (node->is_Gen(GenEnum::gen_wxStdDialogButtonSizer))
                {
                    event_name = "EVT_BUTTON";
                }
                else
                {
                    // remove "wx" prefix
                    event_name.remove_prefix(2);
                    if (event_name == "EVT_CLOSE_WINDOW")
                    {
                        event_name = "EVT_CLOSE";
                    }
                }
                event_set.insert(event_name);
            }
        }
    };

    auto rlambda = [&](Node* node, auto&& rlambda) -> void
    {
        append_perl_events(node);
        if (auto& children = node->get_ChildNodePtrs(); children.size())
        {
            for (const auto& child: children)
            {
                rlambda(child.get(), rlambda);
            }
        }
    };

    if (node->is_Form())
    {
        rlambda(node, rlambda);
    }
    else
    {
        append_perl_events(node);
    }

    wxue::string qw_events;
    if (event_set.size())
    {
        qw_events.RightTrim();
        qw_events.insert(0, "use Wx::Event qw(");
        for (const auto& event: event_set)
        {
            qw_events << event << ' ';
        }
        qw_events.RightTrim();
        qw_events << ");";
    }

    return qw_events;
}
