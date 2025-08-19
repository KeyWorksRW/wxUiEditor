/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common component functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>

#include "gen_common.h"

#include "file_codewriter.h"  // FileCodeWriter -- Classs to write code to disk
#include "gen_results.h"      // Code generation file writing functions
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "lambdas.h"          // Functions for formatting and storage of lamda events
#include "mainapp.h"          // wxGetApp()
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_view_vector -- Read/Write line-oriented strings/files
#include "utils.h"            // Utility functions that work with properties
#include "write_code.h"       // WriteCode -- Write code to Scintilla or file

#include "gen_cpp.h"     // CppCodeGenerator -- Generate C++ code
#include "gen_perl.h"    // PerlCodeGenerator class
#include "gen_python.h"  // PythonCodeGenerator -- Generate wxPython code
#include "gen_ruby.h"    // RubyCodeGenerator -- Generate wxRuby code
#include "gen_rust.h"    // RustCodeGenerator -- Generate wxRust code
#include "gen_xrc.h"     // XrcGenerator -- Generate XRC code

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
    if (!code.hasValue(prop_name))
    {
        code.Add("wxNullColour");
    }
    else
    {
        auto colour = code.node()->as_wxColour(prop_name);
        code.Object("wxColour").QuotedString(colour) += ')';
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
            // While this may not be necessary for non-Windows systems, it does ensure the code
            // compiles on all platforms.
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
    if (node->hasValue(prop_name))
    {
        return GenerateQuotedString(node->as_string(prop_name));
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
    type_panel,
    type_ribbonpanel,
    type_simplebook,
    type_splitter,
    type_wizardpagesimple,

};

// clang-format on

tt_string GetParentName(Node* node, GenLang language)
{
    auto parent = node->getParent();
    while (parent)
    {
        if (parent->isSizer())
        {
            if (parent->isStaticBoxSizer())
            {
                return (tt_string() << parent->getNodeName(language) << "->GetStaticBox()");
            }
        }
        if (parent->isForm())
        {
            return tt_string("this");
        }

        for (auto iter: s_GenParentTypes)
        {
            if (parent->isType(iter))
            {
                tt_string name = parent->getNodeName(language);
                if (parent->isGen(gen_wxCollapsiblePane))
                {
                    name << "->GetPane()";
                }
                return name;
            }
        }
        parent = parent->getParent();
    }

    ASSERT_MSG(parent, tt_string() << node->getNodeName() << " has no parent!");
    return tt_string("internal error");
}

static void GenStyle(Node* node, tt_string& code, const char* prefix)
{
    tt_string all_styles;

    if (node->hasValue(prop_tab_position) &&
        !node->as_string(prop_tab_position).is_sameas("wxBK_DEFAULT"))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->as_string(prop_tab_position);
    }

    if (node->hasValue(prop_orientation) &&
        !node->as_string(prop_orientation).is_sameas("wxGA_HORIZONTAL"))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->as_string(prop_orientation);
    }

    if (node->isGen(gen_wxRichTextCtrl))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << "wxRE_MULTILINE";
    }

    if (node->hasValue(prop_style))
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

    if (node->hasValue(prop_window_style))
    {
        if (all_styles.size())
            all_styles << '|';
        all_styles << node->as_string(prop_window_style);
    }

    if (node->isGen(gen_wxListView))
    {
        if (all_styles.size())
            all_styles << '|';
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

int GetStyleInt(Node* node, const char* prefix)
{
    tt_string styles;

    // If prefix is non-null, this will convert friendly names to wxWidgets constants
    GenStyle(node, styles, prefix);

    int result = 0;
    // Can't use multiview because getConstantAsInt() searches an unordered_map which requires a
    // std::string to pass to it
    tt_string_vector mstr(styles, '|');
    for (auto& iter: mstr)
    {
        // Friendly names will have already been converted, so normal lookup works fine.
        result |= NodeCreation.getConstantAsInt(iter);
    }
    return result;
}

int GetBitlistInt(Node* node, GenEnum::PropName prop_name)
{
    int result = 0;
    // Can't use multiview because getConstantAsInt() searches an unordered_map which requires a
    // std::string to pass to it
    tt_string_vector mstr(node->as_string(prop_name), '|');
    for (auto& iter: mstr)
    {
        // Friendly names will have already been converted, so normal lookup works fine.
        result |= NodeCreation.getConstantAsInt(iter);
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
    bool has_additional_bitmaps =
        (node->hasValue(prop_disabled_bmp) || node->hasValue(prop_pressed_bmp) ||
         node->hasValue(prop_focus_bmp) || node->hasValue(prop_current));
    if (code.size())
        code << '\n';

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
        if (node->hasValue(iter.prop_name))
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
                code << bundle_code.c_str() + (has_additional_bitmaps ? 1 : 0);
                code << "\t" << node->getNodeName() << "->" << iter.function_name;
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
                code << "\t" << node->getNodeName() << "->" << iter.function_name << "("
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
        name.Replace(".", "_", true);  // wxFormBuilder writes files with the extra dots that have
                                       // to be converted to '_'

        if (parts[IndexType].starts_with("Embed"))
        {
            auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
            if (embed)
            {
                name = "wxue_img::" + embed->imgs[0].array_name;
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

        // Note that current documentation states that the client is required, but the header file
        // says otherwise
        if (art_client.size())
            code << art_client;

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

    else if (parts[IndexType].is_sameas("XPM"))
    {
        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            // We get here if there is an Image form that contains the function to retrieve this
            // bundle.
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

            // This should never happen, but if it does, at least generate something that will
            // compiler
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
            // We get here if there is an Image form that contains the function to retrieve this
            // bundle.
            code << function_name;
            return false;
        }

        auto embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
        if (!embed)
        {
            MSG_WARNING(tt_string() << description << " not embedded!");
            code << "wxNullBitmap";
            return false;
        }

        wxSize svg_size { -1, -1 };
        if (parts[IndexSize].size())
        {
            svg_size = GetSizeInfo(parts[IndexSize]);
        }

        tt_string name = "wxue_img::" + embed->imgs[0].array_name;
        code << "wxueBundleSVG(" << name << ", "
             << (to_size_t) (embed->imgs[0].array_size & 0xFFFFFFFF) << ", ";
        code << (to_size_t) (embed->imgs[0].array_size >> 32) << ", wxSize(" << svg_size.x << ", "
             << svg_size.y << "))";
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

        if (auto bundle = ProjectImages.GetPropertyImageBundle(description); bundle)
        {
            if (bundle->lst_filenames.size() == 1)
            {
                tt_string name(bundle->lst_filenames[0].filename());

                if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                {
                    if (auto function_name = ProjectImages.GetBundleFuncName(embed);
                        function_name.size())
                    {
                        code << function_name;
                        return false;
                    }

                    name = "wxue_img::" + embed->imgs[0].array_name;
                }

                code << "wxBitmapBundle::FromBitmap(wxueImage(";
                code << name << ", sizeof(" << name << ")))";
            }
            else if (bundle->lst_filenames.size() == 2)
            {
                tt_string first_name, second_name;
                tt_string first_function, second_function;

                if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[0]); embed)
                {
                    first_function = ProjectImages.GetBundleFuncName(embed);
                    first_name = "wxue_img::" + embed->imgs[0].array_name;
                }
                if (auto embed = ProjectImages.GetEmbeddedImage(bundle->lst_filenames[1]); embed)
                {
                    second_function = ProjectImages.GetBundleFuncName(embed);
                    second_name = "wxue_img::" + embed->imgs[0].array_name;
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
                tt_string name, function;

                code << "{\n\twxVector<wxBitmap> bitmaps;\n";
                for (auto& iter: bundle->lst_filenames)
                {
                    if (auto embed = ProjectImages.GetEmbeddedImage(iter); embed)
                    {
                        function = ProjectImages.GetBundleFuncName(embed);
                        name = "wxue_img::" + embed->imgs[0].array_name;
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
                FAIL_MSG(tt_string(description) << " was not converted to a bundle ahead of time!")

                // This should never happen, but if it does, at least generate something that will
                // compile
                code << "wxNullBitmap";
            }
        }
        else
        {
            FAIL_MSG(tt_string(description) << " was not converted to a bundle ahead of time!")

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
    if (!node->isGen(gen_PanelForm) && !node->isGen(gen_wxToolBar))
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

    if (code.hasValue(prop_window_extra_style))
    {
        code.Eol(eol_if_needed).FormFunction("SetExtraStyle(").FormFunction("GetExtraStyle() | ");
        code.Add(prop_window_extra_style).EndFunction();
    }

    code.GenFontColourSettings();
}

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
// Used when generating code.
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
    auto& get_name = node->as_string(prop_get_function);
    auto& set_name = node->as_string(prop_set_function);
    if (get_name.empty() && set_name.empty())
        return {};

    if (auto& var_name = node->as_string(prop_validator_variable); var_name.size())
    {
        auto val_data_type = node->getValidatorDataType();
        if (val_data_type.empty())
            return {};
        tt_string code;
        if (val_data_type == "wxString" || val_data_type == "wxFileName" ||
            val_data_type == "wxArrayInt")
        {
            if (get_name.size())
                code << "\tconst " << val_data_type << "& " << get_name << "() const { return "
                     << var_name << "; }";
            if (set_name.size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << set_name << "(const " << val_data_type << "& value) { "
                     << var_name << " = value; }";
            }
            if (code.empty())
                return {};
            return code;
        }
        else
        {
            if (get_name.size())
                code << '\t' << val_data_type << ' ' << get_name << "() const { return " << var_name
                     << "; }";
            if (set_name.size())
            {
                if (code.size())
                    code << "\n";
                code << "\tvoid " << set_name << "(" << val_data_type << " value) { " << var_name
                     << " = value; }";
            }
            if (code.empty())
                return {};
            return code;
        }
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
        return;

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
        if (node->isGen(gen_StaticCheckboxBoxSizer))
            var_name = "@" + node->as_string(prop_checkbox_var_name);
        else if (node->isGen(gen_StaticRadioBtnBoxSizer))
            var_name = "@" + node->as_string(prop_radiobtn_var_name);
        else
            var_name.insert(0, "@");
        code.Str("# to retrieve: ").Str(var_name).Str(" = ");
        code.NodeName().Str(".get_validator.get_value()").Eol();
        code.Str(var_name).Str(" = nil").Eol();
        code.NodeName();
    }

    if (code.is_cpp())
    {
        if (node->isGen(gen_StaticCheckboxBoxSizer))
            code.Add(prop_checkbox_var_name);
        else if (node->isGen(gen_StaticRadioBtnBoxSizer))
            code.Add(prop_radiobtn_var_name);
        else
            code.NodeName();
    }

    if (node->isGen(gen_wxRearrangeCtrl))
        code.Function("GetList()");

    code.Function("SetValidator(");

    if (!node->hasProp(prop_validator_data_type))
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

    auto& data_type = node->as_string(prop_validator_data_type);
    ASSERT(data_type.size())
    if (data_type.empty())
    {  // theoretically impossible
        FAIL_MSG(tt_string() << "No validator data type for " << node->getNodeName());
        code.Add("wxDefaultValidator").EndFunction();
        return;
    }
    auto style = node->as_string(prop_validator_style);
    style.Replace("wxFILTER_NONE", "", true);
    if (style.starts_with('|'))
        style.erase(0, 1);

    auto validator_type = node->getValidatorType();

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
                    code << '(';
            }
            else
            {
                code.Add(validator_type).Str(".new");
                if (style.size())
                    code << '(';
            }
        }
    }
    else
    {
        if (code.is_ruby() && validator_type == "wxFloatingPointValidator")
            code.Str("Wx::FloatValidator").AddIfRuby(".new") << '(';
        else
            code.Add(validator_type).AddIfRuby(".new") << '(';
    }

    tt_string_vector styles(style, '|', tt::TRIM::both);
    if (validator_type.is_sameas("wxTextValidator"))
    {
        if (style.contains("wxFILTER_"))
        {
            tt_string filters;
            for (auto& iter: styles)
            {
                if (iter.starts_with("wxFILTER_"))
                {
                    if (filters.size())
                        filters << '|';
                    filters << iter;
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
        if (validator_type.is_sameas("wxFloatingPointValidator"))
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
            tt_string num_styles;
            for (auto& iter: styles)
            {
                if (iter.starts_with("wxNUM_"))
                {
                    if (num_styles.size())
                        num_styles << '|';
                    num_styles << iter;
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
    if (validator_type.is_sameas("wxIntegerValidator") ||
        validator_type.is_sameas("wxFloatingPointValidator"))
    {
        if (node->hasValue(prop_minValue))
        {
            if (code.is_cpp())
            {
                code.Eol()
                    .Str("wxStaticCast(")
                    .NodeName()
                    .Str("->GetValidator(), ")
                    .Str(validator_type);
                if (validator_type.is_sameas("wxIntegerValidator"))
                    code.Str("<").Str(data_type).Str(">");
                code.Str(")->SetMin(").Add(prop_minValue).Str(");");
            }
            else if (code.is_ruby())
            {
                code.Eol().NodeName().Str(".get_validator.set_min(").Add(prop_minValue).Str(")");
            }
        }
        if (node->hasValue(prop_maxValue))
        {
            if (code.is_cpp())
            {
                code.Eol()
                    .Str("wxStaticCast(")
                    .NodeName()
                    .Str("->GetValidator(), ")
                    .Str(validator_type);
                if (validator_type.is_sameas("wxIntegerValidator"))
                    code.Str("<").Str(data_type).Str(">");
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
        // In theory, we could create an alpha channel using black as the transparency, but it just
        // doesn't make sense for the user to be using XPM files as an icon.
        code << "// XPM files do not contain an alpha channel and cannot be used as an icon.";
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

        code << "SetIcon(wxArtProvider::GetBitmapBundle(" << art_id << ", ";
        // Note that current documentation states that the client is required, but the header file
        // says otherwise
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

        if (auto function_name = ProjectImages.GetBundleFuncName(description); function_name.size())
        {
            code << "SetIcon(" << function_name;
        }
        else
        {
            tt_string name = "wxue_img::" + embed->imgs[0].array_name;
            code << "SetIcon(wxueBundleSVG(" << name << ", "
                 << (to_size_t) (embed->imgs[0].array_size & 0xFFFFFFFF) << ", ";
            code << (to_size_t) (embed->imgs[0].array_size >> 32) << ", wxSize(" << svg_size.x
                 << ", " << svg_size.y << "))";
        }

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
                        name = "wxue_img::" + embed->imgs[0].array_name;
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
                            name = "wxue_img::" + embed->imgs[0].array_name;
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
            FAIL_MSG(tt_string(description) << " was not converted to a bundle ahead of time!")
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
        (node->hasValue(prop_var_name) &&
         ((node->as_string(prop_class_access) != "none") || node->isGen(gen_tool_dropdown) ||
          (node->isGen(gen_auitool) &&
           node->as_string(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")));

    if (node->as_bool(prop_disabled) ||
        (node->as_string(prop_id) == "wxID_ANY" && node->getInUseEventCount()))
    {
        code.AddAuto().NodeName();
        code += " = ";
        need_variable_result = false;  // make certain we don't add this again
    }

    if (need_variable_result)
    {
        if (node->isLocal())
        {
            code.AddIfCpp("auto* ");
        }
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

    code.QuotedString(prop_label).Comma();

    if (!code.hasValue(prop_bitmap))
    {
        code.Add("wxNullBitmap");
    }
    else
    {
        tt_string_vector parts(node->as_string(prop_bitmap), BMP_PROP_SEPARATOR, tt::TRIM::both);

        if (parts.size() <= 1 || parts[IndexImage].empty())
        {
            code.Add("wxNullBitmap");
        }
        else
        {
            GenerateBundleParameter(code, parts);
        }
    }

    if (!node->hasValue(prop_tooltip) && !node->hasValue(prop_statusbar))
    {
        if (node->isGen(gen_tool_dropdown))
        {
            code.Comma().Add("wxEmptyString").Comma().Add("wxITEM_DROPDOWN");
        }
        else if (node->as_string(prop_kind) != "wxITEM_NORMAL")
        {
            code.Comma().Add("wxEmptyString").Comma().as_string(prop_kind);
        }
    }

    else if (node->hasValue(prop_tooltip) && !node->hasValue(prop_statusbar))
    {
        code.Comma().QuotedString(prop_tooltip);
        if (node->isGen(gen_tool_dropdown))
        {
            code.Comma().Add("wxITEM_DROPDOWN");
        }
        else if (node->as_string(prop_kind) != "wxITEM_NORMAL")
        {
            code.Comma().as_string(prop_kind);
        }
    }

    else if (node->hasValue(prop_statusbar))
    {
        code.Comma().Add("wxNullBitmap").Comma().as_string(prop_kind).Comma();

        code.QuotedString(prop_tooltip).Comma().QuotedString(prop_statusbar);
        if (node->isGen(gen_auitool))
        {
            code.Comma();
            code.AddIfCpp("nullptr");
            code.AddIfPython("None");
            code.AddIfRuby("nil");
        }
    }
    code.EndFunction();
}

bool BitmapList(Code& code, const GenEnum::PropName prop)
{
    auto* node = code.node();  // for convenience
    // Note that Ruby always uses a function, and therefore has no need for a list
    if (!node->hasValue(prop) || code.is_ruby())
    {
        return false;
    }

    auto& description = node->as_string(prop);
    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

    if (parts[IndexImage].empty() || parts[IndexType].contains("Art") ||
        parts[IndexType].contains("SVG"))
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
    bool use_lambda = (node->hasValue(prop_var_name) &&
                       (node->isGen(gen_tool_dropdown) ||
                        (node->isGen(gen_auitool) &&
                         node->as_string(prop_initial_state) != "wxAUI_BUTTON_STATE_NORMAL")));

    if (use_lambda)
    {
        code.Str("auto make_").NodeName() << "_bundle = [&]() -> wxBitmapBundle";
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
                    name = "wxue_img::" + embed->imgs[0].array_name;
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

bool GenerateLanguageForm(Node* form, GenResults& results, std::vector<tt_string>* pClassList,
                          GenLang language)
{
    auto [path, has_base_file] = Project.GetOutputPath(form, language);
    if (!has_base_file)
    {
#if !defined(_DEBUG)
        // For a lot of testing of projects with multiple dialogs, there may only be a
        // few forms where generation is being tested, so don't nag in Debug builds.
        // :-)
        results.msgs.emplace_back()
            << "No filename specified for " << form->as_string(prop_class_name) << '\n';
#endif  // _DEBUG
        return false;
    }

    std::unique_ptr<class BaseCodeGenerator> code_generator;
    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            code_generator = std::make_unique<CppCodeGenerator>(form);
            break;

        case GEN_LANG_PYTHON:
            code_generator = std::make_unique<PythonCodeGenerator>(form);
            break;

        case GEN_LANG_RUBY:
            code_generator = std::make_unique<RubyCodeGenerator>(form);
            break;

        case GEN_LANG_PERL:
            code_generator = std::make_unique<PerlCodeGenerator>(form);
            break;

        case GEN_LANG_RUST:
            code_generator = std::make_unique<RustCodeGenerator>(form);
            break;

#if GENERATE_NEW_LANG_CODE
        case GEN_LANG_FORTRAN:
            code_generator = std::make_unique<FortranCodeGenerator>(form);
            break;

        case GEN_LANG_HASKELL:
            code_generator = std::make_unique<HaskellCodeGenerator>(form);
            break;

        case GEN_LANG_LUA:
            code_generator = std::make_unique<LuaCodeGenerator>(form);
            break;
#endif

        case GEN_LANG_XRC:
            code_generator = std::make_unique<XrcCodeGenerator>(form);
            break;

        default:
            FAIL_MSG("Unknown language!")
            break;
    }

    auto h_cw = std::make_unique<FileCodeWriter>(path);
    if (language == GEN_LANG_RUBY)
    {
        h_cw->SetTabToSpaces(2);
    }
    code_generator->SetHdrWriteCode(h_cw.get());

    path.replace_extension(GetLanguageExtension(language));
    auto cpp_cw = std::make_unique<FileCodeWriter>(path);
    if (language == GEN_LANG_RUBY)
    {
        cpp_cw->SetTabToSpaces(2);
    }
    code_generator->SetSrcWriteCode(cpp_cw.get());

    switch (language)
    {
        case GEN_LANG_CPLUSPLUS:
            code_generator->GenerateClass();
            break;

        case GEN_LANG_PERL:
            code_generator->GenerateClass();
            break;

        case GEN_LANG_PYTHON:
            code_generator->GenerateClass();
            break;

        case GEN_LANG_RUBY:
            code_generator->GenerateClass();
            break;

        case GEN_LANG_RUST:
            code_generator->GenerateClass();
            break;

#if GENERATE_NEW_LANG_CODE
        case GEN_LANG_FORTRAN:
            code_generator->GenerateClass();
            break;

        case GEN_LANG_HASKELL:
            code_generator->GenerateClass();
            break;

        case GEN_LANG_LUA:
            code_generator->GenerateClass();
            break;
#endif  // GENERATE_NEW_LANG_CODE

        case GEN_LANG_XRC:
            code_generator->GenerateClass();
            break;

        default:
            FAIL_MSG("Unknown language!")
            break;
    }

    int flags = flag_no_ui;
    if (pClassList)
        flags |= flag_test_only;
    auto retval = cpp_cw->WriteFile(language, flags, form);

    if (auto warning_msgs = code_generator->getWarnings(); warning_msgs.size())
    {
        for (auto& iter: warning_msgs)
        {
            results.msgs.emplace_back() << iter << '\n';
        }
    }

    if (retval > 0)
    {
        if (!pClassList)
        {
            results.updated_files.emplace_back(path);
        }
        else
        {
            if (form->isGen(gen_Images))
                pClassList->emplace_back(GenEnum::map_GenNames[gen_Images]);
            if (form->isGen(gen_Data))
                pClassList->emplace_back(GenEnum::map_GenNames[gen_Data]);
            else
                pClassList->emplace_back(form->as_string(prop_class_name));
            return true;
        }
    }

    else if (retval < 0)
    {
        results.msgs.emplace_back() << "Cannot create or write to the file " << path << '\n';
    }
    else  // retval == result::exists
    {
        ++results.file_count;
    }
    return true;
}

bool GenerateLanguageFiles(GenResults& results, std::vector<tt_string>* pClassList,
                           GenLang language)
{
    if (Project.getChildCount() == 0)
    {
        wxMessageBox("You cannot generate any code until you have added a top level form.",
                     "Code Generation");
        return false;
    }

    ASSERT_MSG(language != GEN_LANG_NONE, "No language specified for code generation!");

    if (wxGetApp().isTestingMenuEnabled())
        results.StartClock();

    tt_cwd cwd(true);
    Project.ChangeDir();

    bool generate_result = false;

    if (language == GEN_LANG_CPLUSPLUS)
    {
        generate_result = GenerateCppFiles(results, pClassList);
    }
    else if (language == GEN_LANG_XRC)
    {
        generate_result = GenerateXrcFiles(results, pClassList);
    }
    else
    {
        std::vector<Node*> forms;
        Project.CollectForms(forms);

        for (const auto& form: forms)
        {
            GenerateLanguageForm(form, results, pClassList, language);

            if (results.updated_files.size())
            {
                generate_result = true;
            }
        }
    }
    if (wxGetApp().isTestingMenuEnabled())
        results.EndClock();

    return generate_result;
}

void OnGenerateSingleLanguage(GenLang language)
{
    auto form = wxGetMainFrame()->getSelectedNode();
    if (form && !form->isForm())
    {
        form = form->getForm();
    }
    if (!form)
    {
        wxMessageBox("You must select a form before you can generate code.", "Code Generation");
        return;
    }

    GenResults results;
    GenerateLanguageForm(form, results, nullptr, language);

    tt_string msg;
    if (results.updated_files.size())
    {
        if (results.updated_files.size() == 1)
            msg << "1 file was updated";
        else
            msg << results.updated_files.size() << " files were updated";
        msg << '\n';
    }
    else
    {
        msg << "Generated file is current";
    }

    if (results.msgs.size())
    {
        for (auto& iter: results.msgs)
        {
            msg << '\n';
            msg << iter;
        }
    }

    wxMessageBox(msg, tt_string() << GenLangToString(language) << " Code Generation",
                 wxOK | wxICON_INFORMATION);
}

void OnGenerateLanguage(GenLang language)
{
    GenResults results;
    GenerateLanguageFiles(results, nullptr, language);

    tt_string msg;
    if (results.updated_files.size())
    {
        if (results.updated_files.size() == 1)
            msg << "1 file was updated";
        else
            msg << " files were updated";
        msg << '\n';
    }
    else
    {
        msg << "All " << results.file_count << " generated files are current";
    }

    if (results.msgs.size())
    {
        for (auto& iter: results.msgs)
        {
            msg << '\n';
            msg << iter;
        }
    }

    wxMessageBox(msg, tt_string() << GenLangToString(language) << " Code Generation",
                 wxOK | wxICON_INFORMATION);
}

tt_string GatherPerlNodeEvents(Node* node)
{
    std::set<std::string_view> event_set;

    auto append_perl_events = [&](Node* node)
    {
        for (auto& iter: node->getMapEvents())
        {
            // Only add the event if a handler was specified
            if (iter.second.get_value().size())
            {
                std::string_view event_name = iter.first;
                if (node->isGen(GenEnum::gen_wxStdDialogButtonSizer))
                {
                    event_name = "EVT_BUTTON";
                }
                else
                {
                    // remove "wx" prefix
                    event_name.remove_prefix(2);
                    if (event_name == "EVT_CLOSE_WINDOW")
                        event_name = "EVT_CLOSE";
                }
                event_set.insert(event_name);
            }
        }
    };

    auto rlambda = [&](Node* node, auto&& rlambda) -> void
    {
        append_perl_events(node);
        if (auto& children = node->getChildNodePtrs(); children.size())
        {
            for (auto child: children)
            {
                rlambda(child.get(), rlambda);
            }
        }
    };

    if (node->isForm())
    {
        rlambda(node, rlambda);
    }
    else
    {
        append_perl_events(node);
    }

    tt_string qw_events;
    if (event_set.size())
    {
        qw_events.RightTrim();
        qw_events.insert(0, "use Wx::Event qw(");
        for (auto& event: event_set)
        {
            qw_events << event << ' ';
        }
        qw_events.RightTrim();
        qw_events << ");";
    }

    return qw_events;
}
