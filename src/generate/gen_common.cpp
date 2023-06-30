/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common component functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <charconv>  // for std::to_chars

#include "gen_common.h"

#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "image_handler.h"    // ProjectImage class
#include "image_handler.h"    // ImageHandler class
#include "lambdas.h"          // Functions for formatting and storage of lamda events
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // WriteCode -- Write code to Scintilla or file

void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr)
{
    if (node->isPropValue(prop_class_access, "none"))
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
        if (code.PropContains(prop_name, "wx"))
        {
            code.Add("wxSystemSettings").ClassMethod("GetColour(").as_string(prop_name).Str(")");
        }
        else
        {
            auto colour = code.node()->prop_as_wxColour(prop_name);
            code.Add(tt_string().Format("wxColour(%i, %i, %i)", colour.Red(), colour.Green(), colour.Blue()));
        }
    }
}

tt_string GenerateQuotedString(const tt_string& str)
{
    tt_string code;

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
            // While this may not be necessary for non-Windows systems, it does ensure the code compiles on all platforms.
            if (Project.as_bool(prop_internationalize))
                code << "_(wxString::FromUTF8(\"" << str_with_escapes << "\"))";
            else
                code << "wxString::FromUTF8(\"" << str_with_escapes << "\")";
        }
        else
        {
            if (Project.as_bool(prop_internationalize))
                code << "_(\"" << str_with_escapes << "\")";
            else
                code << "\"" << str_with_escapes << "\"";
        }
    }
    else
    {
        code << "wxEmptyString";
    }

    return code;
}

tt_string GenerateQuotedString(Node* node, GenEnum::PropName prop_name)
{
    if (node->HasValue(prop_name))
    {
        return GenerateQuotedString(node->prop_as_string(prop_name));
    }
    else
    {
        return tt_string("wxEmptyString");
    }
}

// clang-format off

// List of valid component parent types
static constexpr GenType s_GenParentTypes[] = {

    type_auinotebook,
    type_bookpage,
    type_choicebook,
    type_container,
    type_listbook,
    type_notebook,
    type_ribbonpanel,
    type_simplebook,
    type_splitter,
    type_wizardpagesimple,

};

// clang-format on

tt_string GetParentName(Node* node)
{
    auto parent = node->GetParent();
    while (parent)
    {
        if (parent->IsSizer())
        {
            if (parent->IsStaticBoxSizer())
            {
                return (tt_string() << parent->get_node_name() << "->GetStaticBox()");
            }
        }
        if (parent->IsForm())
        {
            return tt_string("this");
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->isType(iter))
            {
                tt_string name = parent->get_node_name();
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    name << "->GetPane()";
                }
                return name;
            }
        }
        parent = parent->GetParent();
    }

    ASSERT_MSG(parent, tt_string() << node->get_node_name() << " has no parent!");
    return tt_string("internal error");
}

void GenPos(Node* node, tt_string& code)
{
    auto point = node->prop_as_wxPoint(prop_pos);
    if (point.x != -1 || point.y != -1)
    {
        if (node->prop_as_string(prop_pos).contains("d", tt::CASE::either))
        {
            code << "ConvertDialogToPixels(wxPoint(" << point.x << ", " << point.y << "))";
        }
        else
        {
            code << "wxPoint(" << point.x << ", " << point.y << ")";
        }
    }
    else
        code << "wxDefaultPosition";
}

void GenSize(Node* node, tt_string& code)
{
    if (node->as_wxSize(prop_size) != wxDefaultSize)
        code << GenerateWxSize(node, prop_size);
    else
        code << "wxDefaultSize";
}

void GenStyle(Node* node, tt_string& code, const char* prefix)
{
    tt_string all_styles;

    if (node->HasValue(prop_tab_position) && !node->prop_as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_tab_position);
    }

    if (node->HasValue(prop_orientation) && !node->prop_as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_orientation);
    }

    if (node->isGen(gen_wxRichTextCtrl))
    {
        if (all_styles.size())
            all_styles << '|';
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
            all_styles << node->prop_as_constant(prop_style, prefix);
        }
        else
        {
            all_styles << node->prop_as_string(prop_style);
        }
    }

    if (node->HasValue(prop_window_style))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_window_style);
    }

    if (node->isGen(gen_wxListView))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->prop_as_string(prop_mode);
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

int GetStyleInt(Node* node, const char* prefix)
{
    tt_string styles;

    // If prefix is non-null, this will convert friendly names to wxWidgets constants
    GenStyle(node, styles, prefix);

    int result = 0;
    // Can't use multiview because GetConstantAsInt() searches an unordered_map which requires a std::string to pass to it
    tt_string_vector mstr(styles, '|');
    for (auto& iter: mstr)
    {
        // Friendly names will have already been converted, so normal lookup works fine.
        result |= NodeCreation.GetConstantAsInt(iter);
    }
    return result;
}

int GetBitlistInt(Node* node, GenEnum::PropName prop_name)
{
    int result = 0;
    // Can't use multiview because GetConstantAsInt() searches an unordered_map which requires a std::string to pass to it
    tt_string_vector mstr(node->value(prop_name), '|');
    for (auto& iter: mstr)
    {
        // Friendly names will have already been converted, so normal lookup works fine.
        result |= NodeCreation.GetConstantAsInt(iter);
    }
    return result;
}

struct BTN_BMP_TYPES
{
    GenEnum::PropName prop_name;
    const char* function_name;
};

inline const BTN_BMP_TYPES btn_bmp_types[] = {
    { prop_bitmap, "SetBitmap" },
    { prop_disabled_bmp, "SetBitmapDisabled" },
    { prop_pressed_bmp, "SetBitmapPressed" },
    { prop_focus_bmp, "SetBitmapFocus" },
    { prop_current, "SetBitmapCurrent" },
};

bool GenBtnBimapCode(Node* node, tt_string& code, bool is_single)
{
    bool has_additional_bitmaps = (node->HasValue(prop_disabled_bmp) || node->HasValue(prop_pressed_bmp) ||
                                   node->HasValue(prop_focus_bmp) || node->HasValue(prop_current));
    if (code.size())
        code << '\n';

    bool is_old_widgets = (Project.value(prop_wxWidgets_version) == "3.1");
    if (is_old_widgets)
    {
        if (code.size() && !(code.back() == '\n'))
            code << '\n';
        code << "#if wxCHECK_VERSION(3, 1, 6)\n";
    }

    if (has_additional_bitmaps)
    {
        if (code.size() && !(code.back() == '\n'))
            code << '\n';
        code << "{\n";
    }

    tt_string bundle_code;
    bool is_vector_generated = false;

    for (auto& iter: btn_bmp_types)
    {
        if (node->HasValue(iter.prop_name))
        {
            bundle_code.clear();
            bool is_code_block = GenerateBundleCode(node->prop_as_string(iter.prop_name), bundle_code);
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
                code << bundle_code.c_str() + (has_additional_bitmaps ? 1 : 0);
                code << "\t" << node->get_node_name() << "->" << iter.function_name;
                code << "(wxBitmapBundle::FromBitmaps(bitmaps));";

                if (!has_additional_bitmaps)
                {
                    code << "\n}";
                }
            }
            else
            {
                if (code.size() && !(code.back() == '\n'))
                    code << '\n';
                code << "\t" << node->get_node_name() << "->" << iter.function_name << "(" << bundle_code << ");";
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

    /////////// wxWidgets 3.1 code ///////////

    if (is_old_widgets)
    {
        code << "\n#else\n";
        code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";

        if (node->HasValue(prop_disabled_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapDisabled("
                 << GenerateBitmapCode(node->prop_as_string(prop_disabled_bmp)) << ");";
        }

        if (node->HasValue(prop_pressed_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapPressed("
                 << GenerateBitmapCode(node->prop_as_string(prop_pressed_bmp)) << ");";
        }

        if (node->HasValue(prop_focus_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapFocus(" << GenerateBitmapCode(node->prop_as_string(prop_focus_bmp))
                 << ");";
        }

        if (node->HasValue(prop_current))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapCurrent(" << GenerateBitmapCode(node->prop_as_string(prop_current))
                 << ");";
        }

        code << "\n#endif  // wxCHECK_VERSION(3, 1, 6)";
    }

    return is_vector_generated;
}

tt_string GenerateBitmapCode(const tt_string& description)
{
    tt_string code;

    if (description.empty())
    {
        code << "wxNullBitmap";
        return code;
    }

    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexType].starts_with("SVG"))
    {
        code << "wxNullBitmap /* SVG images require wxWidgets 3.1.6 */";
        return code;
    }
    else if (parts[IndexImage].empty())
    {
        code << "wxNullBitmap";
        return code;
    }

    if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "wxArtProvider::GetBitmap(" << art_id;
        if (art_client.size())
            code << ", " << art_client;
        code << ')';

        return code;
    }

    tt_string result;
    if (parts[IndexType].is_sameas("XPM"))
    {
        code << "wxImage(";

        tt_string name(parts[IndexImage].filename());
        name.remove_extension();
        code << name << "_xpm)";
    }
    else
    {
        code << "wxueImage(";

        tt_string name(parts[1].filename());
        name.remove_extension();
        name.Replace(".", "_", true);  // wxFormBuilder writes files with the extra dots that have to be converted to '_'

        if (parts[IndexType].starts_with("Embed"))
        {
            auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                name = "wxue_img::" + embed->array_name;
            }
        }

        code << name << ", sizeof(" << name << "))";
    }
    return code;
}

bool GenerateBundleCode(const tt_string& description, tt_string& code)
{
    if (description.empty())
    {
        code << "wxNullBitmap";
        return false;
    }

    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty())
    {
        code << "wxNullBitmap";
        return false;
    }

    if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "wxArtProvider::GetBitmapBundle(" << art_id << ", ";

        // Note that current documentation states that the client is required, but the header file says otherwise
        if (art_client.size())
            code << art_client;
        code << ')';
    }

    else if (parts[IndexType].is_sameas("XPM"))
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            // We get here if there is an Image form that contains the function to retrieve this bundle.
            code << function_name;
            return false;
        }

        if (auto bundle = ProjectImages.GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                code << "wxBitmapBundle::FromBitmap(wxImage(";
                tt_string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                code << name << "_xpm))";
            }
            else if (bundle->lst_filenames.size() == 2)
            {
                code << "wxBitmapBundle::FromBitmaps(wxImage(";
                tt_string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                code << name << "_xpm), ";
                name = bundle->lst_filenames[1].filename();
                name.remove_extension();
                code << "wxImage(" << name << "_xpm))";
            }
            else
            {
                code << "{\n\t\t\twxVector<wxBitmap> bitmaps;\n";
                for (auto& iter: bundle->lst_filenames)
                {
                    tt_string name(iter.filename());
                    name.remove_extension();
                    code << "\t\t\tbitmaps.push_back(wxImage(" << name << "_xpm));\n";
                }

                // Return true to indicate a code block was generated
                return true;
            }
        }
        else
        {
            FAIL_MSG(tt_string(description) << " was not converted to a bundle ahead of time!")

            // This should never happen, but if it does, at least generate something that will compiler
            code << "wxImage(";
            tt_string name(parts[IndexImage].filename());
            name.remove_extension();
            code << name << "_xpm)";
        }
    }
    else if (description.starts_with("SVG"))
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            // We get here if there is an Image form that contains the function to retrieve this bundle.
            code << function_name;
            return false;
        }

        auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (!embed)
        {
            FAIL_MSG(tt_string() << description << " not embedded!")
            code << "wxNullBitmap";
            return false;
        }

        wxSize svg_size { -1, -1 };
        if (parts[IndexSize].size())
        {
            svg_size = GetSizeInfo(parts[IndexSize]);
        }

        tt_string name = "wxue_img::" + embed->array_name;
        code << "wxueBundleSVG(" << name << ", " << (embed->array_size & 0xFFFFFFFF) << ", ";
        code << (embed->array_size >> 32) << ", wxSize(" << svg_size.x << ", " << svg_size.y << "))";
    }
    else
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            // We get here if there is an Image form that contains the function to retrieve this bundle.
            code << function_name;
            return false;
        }

        if (auto bundle = ProjectImages.GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                code << "wxBitmapBundle::FromBitmap(wxueImage(";
                tt_string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                name.Replace(".", "_", true);  // fix wxFormBuilder header files

                if (parts[IndexType].starts_with("Embed"))
                {
                    auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                    if (embed)
                    {
                        name = "wxue_img::" + embed->array_name;
                    }
                }

                code << name << ", sizeof(" << name << ")))";
            }
            else if (bundle->lst_filenames.size() == 2)
            {
                code << "wxBitmapBundle::FromBitmaps(wxueImage(";
                tt_string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                name.Replace(".", "_", true);  // fix wxFormBuilder header files

                if (parts[IndexType].starts_with("Embed"))
                {
                    auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                    if (embed)
                    {
                        name = "wxue_img::" + embed->array_name;
                    }
                }
                code << name << ", sizeof(" << name << ")), wxueImage(";

                name = bundle->lst_filenames[1].filename();
                name.remove_extension();
                name.Replace(".", "_", true);

                if (parts[IndexType].starts_with("Embed"))
                {
                    auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]);
                    if (embed)
                    {
                        name = "wxue_img::" + embed->array_name;
                    }
                }
                code << name << ", sizeof(" << name << ")))";
            }
            else
            {
                code << "{\n\t\t\twxVector<wxBitmap> bitmaps;\n";
                for (auto& iter: bundle->lst_filenames)
                {
                    tt_string name(iter.filename());
                    name.remove_extension();
                    name.Replace(".", "_", true);  // fix wxFormBuilder header files
                    if (parts[IndexType].starts_with("Embed"))
                    {
                        auto embed = ProjectImages.GetEmbeddedImage(iter);
                        if (embed)
                        {
                            name = "wxue_img::" + embed->array_name;
                        }
                    }
                    code << "\t\t\tbitmaps.push_back(wxueImage(" << name << ", sizeof(" << name << ")));\n";
                }

                // Return true to indicate a code block was generated
                return true;
            }
        }
        else
        {
            FAIL_MSG(tt_string(description) << " was not converted to a bundle ahead of time!")

            // This should never happen, but if it does, at least generate something that will compiler
            code << "wxNullBitmsap";
        }
    }

    return false;
}

bool GenerateVectorCode(const tt_string& description, tt_string& code)
{
    if (description.empty())
    {
        return false;
    }

    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty() || parts[IndexType].contains("Art") || parts[IndexType].contains("SVG"))
    {
        return false;
    }

    auto bundle = ProjectImages.GetPropertyImageBundle(description);

    if (!bundle || bundle->lst_filenames.size() < 3)
    {
        return false;
    }

    // If we get here, then we need to first put the bitmaps into a wxVector in order for wxBitmapBundle to load them.

    code << "{\n";
    if (Project.value(prop_wxWidgets_version) == "3.1")
    {
        code << "#if wxCHECK_VERSION(3, 1, 6)\n";
    }
    code << "\twxVector<wxBitmap> bitmaps;\n";

    bool is_xpm = (parts[IndexType].is_sameas("XPM"));

    for (auto& iter: bundle->lst_filenames)
    {
        tt_string name(iter.filename());
        name.remove_extension();
        if (is_xpm)
        {
            code << "\tbitmaps.push_back(wxImage(" << name << "_xpm));\n";
        }
        else
        {
            name.Replace(".", "_", true);  // fix wxFormBuilder header files
            if (parts[IndexType].starts_with("Embed"))
            {
                auto embed = ProjectImages.GetEmbeddedImage(iter);
                if (embed)
                {
                    name = "wxue_img::" + embed->array_name;
                }
            }
            code << "\tbitmaps.push_back(wxueImage(" << name << ", sizeof(" << name << ")));\n";
        }
    }
    if (Project.value(prop_wxWidgets_version) == "3.1")
    {
        code << "#endif\n";
    }

    // Note that the opening brace is *not* closed. That will have to be handled by BaseCodeGenerator::GenConstruction()

    return true;
}

void GenFormSettings(Code& code)
{
    const auto* node = code.node();
    if (!node->isGen(gen_PanelForm) && !node->isGen(gen_wxToolBar))
    {
        const auto max_size = node->prop_as_wxSize(prop_maximum_size);
        const auto min_size = node->prop_as_wxSize(prop_minimum_size);
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

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string. Used when generating
// code.
tt_string ConvertToCodeString(const tt_string& text)
{
    tt_string result;

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

std::optional<tt_string> GenGetSetCode(Node* node)
{
    auto& get_name = node->prop_as_string(prop_get_function);
    auto& set_name = node->prop_as_string(prop_set_function);
    if (get_name.empty() && set_name.empty())
        return {};

    if (auto& var_name = node->prop_as_string(prop_validator_variable); var_name.size())
    {
        auto& val_data_type = node->prop_as_string(prop_validator_data_type);
        if (val_data_type.empty())
            return {};
        tt_string code;
        if (val_data_type == "wxString" || val_data_type == "wxFileName" || val_data_type == "wxArrayInt")
        {
            if (get_name.size())
                code << "\tconst " << val_data_type << "& " << get_name << "() const { return " << var_name << "; }";
            if (set_name.size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << set_name << "(const " << val_data_type << "& value) { " << var_name << " = value; }";
            }
            if (code.empty())
                return {};
            return code;
        }
        else
        {
            if (get_name.size())
                code << '\t' << val_data_type << ' ' << get_name << "() const { return " << var_name << "; }";
            if (set_name.size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << set_name << "(" << val_data_type << " value) { " << var_name << " = value; }";
            }
            if (code.empty())
                return {};
            return code;
        }
    }

    return {};
}

std::optional<tt_string> GenValidatorSettings(Node* node)
{
    if (auto& var_name = node->prop_as_string(prop_validator_variable); var_name.size())
    {
        auto& val_data_type = node->prop_as_string(prop_validator_data_type);
        if (val_data_type.empty())
            return {};

        tt_string code;
        auto& validator_type = node->prop_as_string(prop_validator_type);
        if (validator_type.is_sameas("wxTextValidator"))
        {
            code << node->get_node_name() << "->SetValidator(wxTextValidator(" << node->prop_as_string(prop_validator_style)
                 << ", &" << var_name << "));";
        }
        else
        {
            if (node->isGen(gen_StaticCheckboxBoxSizer))
                code << node->prop_as_string(prop_checkbox_var_name);
            else if (node->isGen(gen_StaticRadioBtnBoxSizer))
                code << node->prop_as_string(prop_radiobtn_var_name);
            else
                code << node->get_node_name();

            if (node->isGen(gen_wxRearrangeCtrl))
                code << "->GetList()";
            code << "->SetValidator(wxGenericValidator(&" << var_name << "));";
        }

        return code;
    }

    return {};
}

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate everything needed to set the
// window's icon.

tt_string GenerateIconCode(const tt_string& description)
{
    tt_string code;

    if (description.empty())
    {
        return code;
    }

    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts.size() < 2 || parts[IndexImage].empty())
    {
        return code;
    }

    if (parts[IndexType].is_sameas("XPM"))
    {
        // In theory, we could create an alpha channel using black as the transparency, but it just doesn't make sense
        // for the user to be using XPM files as an icon.
        code << "// XPM files do not contain an alpha channel and cannot be used as an icon.";
        return code;
    }

    if (Project.value(prop_wxWidgets_version) == "3.1" && !parts[IndexType].is_sameas("SVG"))
    {
        code << "#if wxCHECK_VERSION(3, 1, 6)\n";
    }

    if (parts[IndexType].contains("Art"))
    {
        tt_string art_id(parts[IndexArtID]);
        tt_string art_client;
        if (auto pos = art_id.find('|'); tt::is_found(pos))
        {
            art_client = art_id.subview(pos + 1);
            art_id.erase(pos);
        }

        code << "SetIcon(wxArtProvider::GetBitmapBundle(" << art_id << ", ";
        // Note that current documentation states that the client is required, but the header file says otherwise
        if (art_client.size())
            code << art_client;
        code << ").GetIconFor(this));\n";
    }
    else if (description.starts_with("SVG"))
    {
        auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (!embed)
        {
            FAIL_MSG(tt_string() << description << " not embedded!")
            return code;
        }

        auto svg_size = GetSizeInfo(parts[IndexSize]);

        tt_string name = "wxue_img::" + embed->array_name;
        code << "SetIcon(wxueBundleSVG(" << name << ", " << (embed->array_size & 0xFFFFFFFF) << ", ";
        code << (embed->array_size >> 32) << ", wxSize(" << svg_size.x << ", " << svg_size.y << "))";
        code << ".GetIconFor(this));\n";
        return code;
    }
    else
    {
        if (auto bundle = ProjectImages.GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                code << "SetIcon(wxBitmapBundle::FromBitmap(wxueImage(";
                tt_string name(bundle->lst_filenames[0].filename());
                name.remove_extension();
                name.Replace(".", "_", true);  // fix wxFormBuilder header files

                if (parts[IndexType].starts_with("Embed"))
                {
                    auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]);
                    if (embed)
                    {
                        name = "wxue_img::" + embed->array_name;
                    }
                }

                code << name << ", sizeof(" << name << ")))";
                code << ".GetIconFor(this));\n";
            }
            else
            {
                code << "{\n\twxIconBundle icon_bundle;\n\twxIcon icon;\n";
                for (auto& iter: bundle->lst_filenames)
                {
                    tt_string name(iter.filename());
                    name.remove_extension();
                    name.Replace(".", "_", true);  // fix wxFormBuilder header files
                    if (parts[IndexType].starts_with("Embed"))
                    {
                        auto embed = ProjectImages.GetEmbeddedImage(iter);
                        if (embed)
                        {
                            name = "wxue_img::" + embed->array_name;
                        }
                    }
                    code << "\ticon.CopyFromBitmap(wxueImage(" << name << ", sizeof(" << name << ")));\n";
                    code << "\ticon_bundle.AddIcon(icon);\n";
                }
                code << "\tSetIcons(icon_bundle);\n}\n";
            }
        }
        else
        {
            FAIL_MSG(tt_string(description) << " was not converted to a bundle ahead of time!")
            return code;
        }
    }

    if (Project.value(prop_wxWidgets_version) == "3.1")
    {
        code << "#else\n";
        auto image_code = GenerateBitmapCode(description);
        if (!image_code.contains(".Scale") && image_code.starts_with("wxImage("))
        {
            code << "SetIcon(wxIcon(" << image_code.subview(sizeof("wxImage")) << ");\n";
        }
        else
        {
            code << "{\n";
            code << "\twxIcon icon;\n";
            code << "\ticon.CopyFromBitmap(" << GenerateBitmapCode(description) << ");\n";
            code << "\tSetIcon(wxIcon(icon));\n";
            code << "}\n";
        }
        code << "#endif\n";
    }

    return code;
}

tt_string GenerateWxSize(Node* node, PropName prop)
{
    tt_string code;
    auto size = node->prop_as_wxSize(prop);
    if (node->value(prop).contains("d", tt::CASE::either))
    {
        code << "ConvertDialogToPixels(wxSize(" << size.x << ", " << size.y << "))";
    }
    else
    {
        code << "wxSize(" << size.x << ", " << size.y << ")";
    }
    return code;
}

// This is called to add a tool to either wxToolBar or wxAuiToolBar
void GenToolCode(Code& code, const bool is_bitmaps_list)
{
    const auto* node = code.node();
    code.Eol(eol_if_needed);
    if (node->prop_as_bool(prop_disabled) || (node->prop_as_string(prop_id) == "wxID_ANY" && node->GetInUseEventCount()))
    {
        if (node->IsLocal() && code.is_cpp())
            code << "auto* ";
        code.NodeName() << " = ";
    }

    if ((node->IsLocal() && node->isGen(gen_tool_dropdown)) ||
        (node->isGen(gen_auitool) && node->value(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL"))
    {
        code.AddIfCpp("auto* ").NodeName().Add(" = ");
    }
    // If the user doesn't want access, then we have no use for the return value.
    else if (!node->IsLocal())
    {
        code.NodeName().Add(" = ");
    }

    if (node->isParent(gen_wxToolBar) || node->isParent(gen_wxAuiToolBar))
        code.ParentName().Function("AddTool(").as_string(prop_id).Comma();
    else
    {
        if (code.is_python())
            code += "self.";
        code.Add("AddTool(").as_string(prop_id).Comma();
    }
    code.QuotedString(prop_label);
    if (is_bitmaps_list)
    {
        code.Comma();
        if (code.is_cpp())
            code += "wxBitmapBundle::FromBitmaps(bitmaps)";
        else
            code += "wx.BitmapBundle.FromBitmaps(bitmaps)";
    }
    else
    {
        code.Comma();
        if (!code.HasValue(prop_bitmap))
        {
            code.Add("wxNullBitmap");
        }
        else if (code.is_cpp())
        {
            if (Project.value(prop_wxWidgets_version) == "3.1")
            {
                code.Eol() += "#if wxCHECK_VERSION(3, 1, 6)\n\t";
            }

            tt_string bundle_code;
            GenerateBundleCode(node->prop_as_string(prop_bitmap), bundle_code);
            code.CheckLineLength(bundle_code.size());
            code += bundle_code;

            if (Project.value(prop_wxWidgets_version) == "3.1")
            {
                code.Eol() += "#else\n\t";
                code << "wxBitmap(" << GenerateBitmapCode(node->as_string(prop_bitmap)) << ")";
                code.Eol() += "#endif";
                code.Eol();
            }
            else
            {
                code.CheckLineLength();
            }
        }
        else
        {
            PythonBundleCode(code, prop_bitmap);
        }
    }

    if (!node->HasValue(prop_tooltip) && !node->HasValue(prop_statusbar))
    {
        if (node->isGen(gen_tool_dropdown))
        {
            code.Comma().Add("wxEmptyString").Comma().Add("wxITEM_DROPDOWN");
        }
        else if (node->prop_as_string(prop_kind) != "wxITEM_NORMAL")
        {
            code.Comma().Add("wxEmptyString").Comma().as_string(prop_kind);
        }
    }

    else if (node->HasValue(prop_tooltip) && !node->HasValue(prop_statusbar))
    {
        code.Comma().QuotedString(prop_tooltip);
        if (node->isGen(gen_tool_dropdown))
        {
            code.Comma().Add("wxITEM_DROPDOWN");
        }
        else if (node->prop_as_string(prop_kind) != "wxITEM_NORMAL")
        {
            code.Comma().as_string(prop_kind);
        }
    }

    else if (node->HasValue(prop_statusbar))
    {
        code.Comma().Add("wxNullBitmap").Comma().as_string(prop_kind).Comma();

        code.QuotedString(prop_tooltip).Comma().QuotedString(prop_statusbar);
        if (node->isGen(gen_auitool))
        {
            code.Comma();
            code.AddIfCpp("nullptr");
            code.AddIfPython("None");
        }
    }
    code.EndFunction();
}

bool BitmapList(Code& code, const GenEnum::PropName prop)
{
    if (!code.node()->HasValue(prop))
    {
        return false;
    }

    auto& description = code.node()->as_string(prop);
    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty() || parts[IndexType].contains("Art") || parts[IndexType].contains("SVG"))
    {
        return false;
    }

    auto bundle = ProjectImages.GetPropertyImageBundle(description);

    if (!bundle || bundle->lst_filenames.size() < 3)
    {
        return false;
    }

    // If we get here, then the bitmaps need to be put into a vector

    bool is_xpm = (parts[IndexType].is_sameas("XPM"));

    if (code.is_python())
    {
        auto path = MakePythonPath(code.node());

        code += "bitmaps = [ ";
        bool needs_comma = false;
        for (auto& iter: bundle->lst_filenames)
        {
            if (needs_comma)
            {
                code.UpdateBreakAt();
                code.Comma(false).Eol().Tab(3);
            }
            tt_string name(iter);
            name.make_absolute();
            name.make_relative(path);
            name.backslashestoforward();

            code.Str("wxBitmap(").QuotedString(name);
            if (is_xpm)
                code.Comma().Str("wx.BITMAP_TYPE_XPM");
            code += ")";
            needs_comma = true;
        }
        code += " ]\n";
        code.UpdateBreakAt();

        return true;
    }

    //////////////// C++ code starts here ////////////////
    if (Project.value(prop_wxWidgets_version) == "3.1")
    {
        code.Add("#if wxCHECK_VERSION(3, 1, 6)");
    }
    code.OpenBrace().Add("wxVector<wxBitmap> bitmaps;");

    for (auto& iter: bundle->lst_filenames)
    {
        tt_string name(iter.filename());
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
                auto embed = ProjectImages.GetEmbeddedImage(iter);
                if (embed)
                {
                    name = "wxue_img::" + embed->array_name;
                }
            }
            code.Eol().Str("bitmaps.push_back(wxueImage(") << name << ", sizeof(" << name << ")));";
        }
    }
    code.Eol();

    // Caller should add the function that uses the bitmaps, add the closing brace, and if
    // prop_wxWidgets_version == 3.1, follow this with a #else and the alternate code.

    return true;
}
