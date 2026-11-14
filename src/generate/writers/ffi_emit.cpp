/////////////////////////////////////////////////////////////////////////////
// Purpose:   Emit the body of a kwxFFI form from a schema-1 descriptor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*
    The FFI writers share everything except the module scaffold. This file owns the statements
    inside the C3 creation entry:

        form creation -> children -> sizer attach -> std-button sizer -> font/bitmap -> events

    Every call is resolved through Code's descriptor primitives, so a role the descriptor does
    not describe produces exactly one diagnostic and no emitted line. No statement here emits
    an application entry, an event loop or SetTopWindow (C3).
*/

#include "ffi_emit.h"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <vector>

#include <wx/file.h>
#include <wx/utils.h>  // wxGetEnv

#include "code.h"                        // Code -- descriptor-driven primitives
#include "embed_image.h"                 // EmbeddedImage::base_image().array_name
#include "ffi_ui.h"                      // ResolveRole, GetFfiUiDescriptor
#include "image_handler.h"               // ProjectImages
#include "node.h"                        // Node class
#include "project_handler.h"             // ProjectHandler
#include "write_code.h"                  // WriteCode
#include "wxue_namespace/wxue_string.h"  // wxue::StringVector

using namespace GenEnum;

// The derived-type name of a wrapper object's local. Fortran classes follow the descriptor's
// documented `<class>_t` rule; a class the descriptor does not publish (wxFont) uses it too.
std::string GetFortranLocalType(GenLang language, std::string_view wx_class)
{
    if (const FfiUiDescriptor* descriptor = GetFfiUiDescriptor(language); descriptor != nullptr)
    {
        const FfiUiClassMap::const_iterator class_iter = descriptor->classes.find(wx_class);
        if (class_iter != descriptor->classes.end() && !class_iter->second.type.empty())
        {
            return class_iter->second.type;
        }
    }
    std::string name(wx_class);
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char character)
                   {
                       return static_cast<char>(std::tolower(character));
                   });
    return name + "_t";
}

namespace ffi_emit_detail
{
    std::vector<FfiSourceMapEntry>& SourceMapStorage()
    {
        static std::vector<FfiSourceMapEntry> entries;
        return entries;
    }

    struct EmitContext
    {
        Code& code;
        WriteCode* writer;
        Node* form;
        const FfiFormInfo& info;
        GenLang language;
        std::string form_receiver;
        std::string declarations;
        std::string handlers;
        bool ignored_logical_declared { false };
        bool ignored_integer_declared { false };
        bool ok { true };
    };

    // Fortran uses IOR for a flag mask; every other backend joins with the bitwise-or operator.
    [[nodiscard]] std::string CombineFlags(GenLang language, const std::vector<std::string>& parts)
    {
        if (parts.empty())
        {
            return {};
        }
        if (language != GenLang::fortran)
        {
            // LuaJIT is Lua 5.1, which has no `|` operator. The port's own constant table joins
            // masks with `+` (`lua/wx/init.lua`: DEFAULT_DIALOG_STYLE = CAPTION + SYSTEM_MENU +
            // CLOSE_BOX), and every wx flag mask is a disjoint set of bits, so addition composes
            // them the same way the C++ bitwise-or does.
            const std::string_view separator = language == GenLang::luajit ? " + " : " | ";
            std::string joined = parts.front();
            for (size_t index = 1; index < parts.size(); ++index)
            {
                joined += separator;
                joined += parts[index];
            }
            return joined;
        }
        std::string joined = parts.front();
        for (size_t index = 1; index < parts.size(); ++index)
        {
            joined = "ior(" + joined + ", " + parts[index] + ")";
        }
        return joined;
    }

    // The derived-type name of a wrapper object's local, resolved by GetFortranLocalType().

    // Fortran cannot discard a function result: a side-effect call to a wrapper emitted as a
    // function assigns it to a scratch variable of the declared kind.
    [[nodiscard]] std::string FortranScratch(EmitContext& ctx, const std::string& returns)
    {
        if (returns.contains("Bool") || returns == "bool")
        {
            if (!ctx.ignored_logical_declared)
            {
                ctx.declarations += "    logical :: ignored_result\n";
                ctx.ignored_logical_declared = true;
            }
            return "ignored_result";
        }
        if (returns.contains("int") || returns.contains("long") || returns.contains("size"))
        {
            if (!ctx.ignored_integer_declared)
            {
                ctx.declarations += "    integer :: ignored_status\n";
                ctx.ignored_integer_declared = true;
            }
            return "ignored_status";
        }
        return {};
    }

    // Emits one side-effect call: CALL for a module subroutine, an assignment for a module
    // function whose result the form does not use.
    void WriteStatement(EmitContext& ctx, Code& statement, const std::string& node_class,
                        const std::string& var_name, std::string_view property,
                        std::string_view role);

    void EmitMethodStatement(EmitContext& ctx, Node* node, std::string_view wx_class,
                             std::string_view role, const std::string& receiver,
                             const std::string& args, const std::string& node_class,
                             const std::string& var_name)
    {
        Code statement(node, ctx.language);
        if (ctx.language == GenLang::fortran)
        {
            FfiUiCall call;
            if (!ctx.code.FfiResolve(wx_class, role, call))
            {
                ctx.ok = false;
                return;
            }
            if (!call.returns.empty())
            {
                const std::string scratch = FortranScratch(ctx, call.returns);
                if (!scratch.empty())
                {
                    statement.Str(scratch).Str(" = ");
                }
            }
        }
        if (!statement.FfiMethod(wx_class, role, receiver, args))
        {
            ctx.ok = false;
            return;
        }
        WriteStatement(ctx, statement, node_class, var_name, role, role);
    }

    [[nodiscard]] std::string Trim(std::string_view text)
    {
        size_t start = 0;
        while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])) != 0)
        {
            ++start;
        }
        size_t end = text.size();
        while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0)
        {
            --end;
        }
        return std::string(text.substr(start, end - start));
    }

    [[nodiscard]] std::vector<std::string> SplitTokens(std::string_view name)
    {
        std::vector<std::string> tokens;
        std::string current;
        for (const char character: name)
        {
            if (character == '_' || character == '-' || character == '.')
            {
                if (!current.empty())
                {
                    tokens.emplace_back(current);
                    current.clear();
                }
                continue;
            }
            if (std::isupper(static_cast<unsigned char>(character)) != 0 && !current.empty())
            {
                tokens.emplace_back(current);
                current.clear();
            }
            current += static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        if (!current.empty())
        {
            tokens.emplace_back(current);
        }

        // Expand the two spellings the C3 table treats as compound identifiers.
        std::vector<std::string> expanded;
        for (const std::string& token: tokens)
        {
            if (token == "mainframe")
            {
                expanded.emplace_back("main");
                expanded.emplace_back("frame");
            }
            else if (token == "dlg")
            {
                expanded.emplace_back("dialog");
            }
            else
            {
                expanded.emplace_back(token);
            }
        }
        return expanded;
    }

    [[nodiscard]] std::string JoinSnake(const std::vector<std::string>& tokens)
    {
        std::string result;
        for (const std::string& token: tokens)
        {
            if (!result.empty())
            {
                result += '_';
            }
            result += token;
        }
        return result;
    }

    [[nodiscard]] std::string JoinPascal(const std::vector<std::string>& tokens)
    {
        std::string result;
        for (const std::string& token: tokens)
        {
            if (token.empty())
            {
                continue;
            }
            result += static_cast<char>(std::toupper(static_cast<unsigned char>(token[0])));
            result += token.substr(1);
        }
        return result;
    }

    [[nodiscard]] bool IsSnakeLanguage(GenLang language)
    {
        return language == GenLang::julia || language == GenLang::luajit ||
               language == GenLang::rust || language == GenLang::fortran;
    }

    [[nodiscard]] std::string MakeIdentifier(std::string_view raw_name, GenLang language)
    {
        std::string name(raw_name);
        if (name.starts_with("m_"))
        {
            name.erase(0, 2);
        }
        const std::vector<std::string> tokens = SplitTokens(name);
        if (IsSnakeLanguage(language))
        {
            return JoinSnake(tokens);
        }
        std::string camel = JoinPascal(tokens);
        if (!camel.empty())
        {
            camel[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(camel[0])));
        }
        return camel;
    }

    [[nodiscard]] std::string DefaultVarName(Node* node)
    {
        const std::string class_name(node->get_DeclName());
        if (class_name == "wxTextCtrl")
        {
            return "text_ctrl";
        }
        if (class_name == "wxBoxSizer" || class_name.ends_with("Sizer"))
        {
            return "sizer";
        }
        if (class_name == "wxStaticBitmap")
        {
            return "bitmap";
        }
        if (class_name == "wxStdDialogButtonSizer")
        {
            return "std_btn";
        }
        return class_name;
    }

    [[nodiscard]] std::string NodeVarName(Node* node, GenLang language)
    {
        if (node->HasValue(prop_var_name) && !node->as_string(prop_var_name).empty())
        {
            return MakeIdentifier(node->as_string(prop_var_name), language);
        }
        return MakeIdentifier(DefaultVarName(node), language);
    }

    [[nodiscard]] bool HasRole(GenLang language, std::string_view wx_class, std::string_view role)
    {
        const FfiUiDescriptor* descriptor = GetFfiUiDescriptor(language);
        if (!descriptor)
        {
            return false;
        }
        FfiUiCall call;
        return ResolveRole(*descriptor, wx_class, role, call);
    }

    [[nodiscard]] std::string RenderConstants(const Code& code, std::string_view value)
    {
        std::vector<std::string> parts;
        size_t start = 0;
        while (start <= value.size())
        {
            const size_t end = value.find('|', start);
            const std::string token = Trim(value.substr(
                start, end == std::string_view::npos ? std::string_view::npos : end - start));
            // A style property can carry the bare language prefix when no flag is set; it names
            // no constant, so it must not produce `wx()`.
            if (!token.empty() && token != "wx")
            {
                if (token.starts_with("wx") && token.size() > 2)
                {
                    parts.emplace_back(code.FfiConstant(std::string_view(token).substr(2)));
                }
                else
                {
                    parts.emplace_back(token);
                }
            }
            if (end == std::string_view::npos)
            {
                break;
            }
            start = end + 1;
        }
        return CombineFlags(code.get_language(), parts);
    }

    [[nodiscard]] std::string StringArg(const Code& code, std::string_view text)
    {
        if (code.FfiStringTakesPointer())
        {
            return "createWxString(" + code.FfiStringLiteral(text) + ").ptr";
        }
        return code.FfiStringLiteral(text);
    }

    [[nodiscard]] std::string DefaultStyleName(std::string_view wx_class)
    {
        if (wx_class == "wxFrame")
        {
            return "DEFAULT_FRAME_STYLE";
        }
        if (wx_class == "wxDialog")
        {
            return "DEFAULT_DIALOG_STYLE";
        }
        return {};
    }

    [[nodiscard]] std::string BitmapTypeName(std::string_view filename)
    {
        const size_t dot_position = filename.rfind('.');
        std::string extension = dot_position == std::string_view::npos ?
                                    std::string() :
                                    std::string(filename.substr(dot_position + 1));
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char ch)
                       {
                           return static_cast<char>(std::tolower(ch));
                       });
        if (extension == "png")
        {
            return "BITMAP_TYPE_PNG";
        }
        if (extension == "jpg" || extension == "jpeg")
        {
            return "BITMAP_TYPE_JPEG";
        }
        if (extension == "bmp")
        {
            return "BITMAP_TYPE_BMP";
        }
        if (extension == "gif")
        {
            return "BITMAP_TYPE_GIF";
        }
        if (extension == "ico")
        {
            return "BITMAP_TYPE_ICO";
        }
        return "BITMAP_TYPE_ANY";
    }

    // Defined below; declared here because the bitmap expression uses it.
    [[nodiscard]] std::string AsHandle(const Code& code, std::string_view expression);

    // Builds `bitmap.from_base64(...)` through the descriptor and returns the expression.
    [[nodiscard]] std::string BitmapExpr(EmitContext& ctx, Node* node)
    {
        const std::string bitmap_value = node->as_string(prop_bitmap);
        if (bitmap_value.empty())
        {
            return {};
        }

        const size_t semicolon = bitmap_value.find(';');
        const std::string filename =
            semicolon == std::string::npos ? bitmap_value : bitmap_value.substr(semicolon + 1);
        if (semicolon == std::string::npos)
        {
            return {};
        }

        // The embedded image's base64 constant is emitted by WriteImageConstruction under the
        // image's array_name + "_b64"; reuse that name instead of re-deriving it from the
        // filename, which would not match for a name that needs sanitising.
        std::string b64_name;
        if (const wxue::StringVector parts(bitmap_value, BMP_PROP_SEPARATOR, wxue::TRIM::both);
            parts.size() > IndexImage)
        {
            if (const EmbeddedImage* embed = ProjectImages.GetEmbeddedImage(parts[IndexImage]);
                embed != nullptr)
            {
                b64_name = embed->base_image().array_name + "_b64";
            }
        }
        if (b64_name.empty())
        {
            std::string stem = filename;
            const size_t dot_position = stem.rfind('.');
            if (dot_position != std::string::npos)
            {
                stem.erase(dot_position);
            }
            std::replace(stem.begin(), stem.end(), '.', '_');
            std::replace(stem.begin(), stem.end(), '-', '_');
            b64_name = stem + "_b64";
        }
        const std::string bitmap_type = ctx.code.FfiConstant(BitmapTypeName(filename));

        std::string args;
        switch (ctx.language)
        {
            case GenLang::go:
                args = "unsafe.Pointer(unsafe.StringData(" + b64_name + ")), len(" + b64_name +
                       "), " + bitmap_type;
                break;
            case GenLang::typescript:
                {
                    // The wrapper takes a pointer to the raw base64 bytes and their length as the
                    // `usize` bigint parameter. Pin a UTF-8 view of the emitted string constant.
                    const std::string buffer_name =
                        MakeIdentifier(NodeVarName(node, ctx.language) + "_b64", ctx.language);
                    ctx.writer->writeLine("const " + buffer_name + " = new TextEncoder().encode(" +
                                          b64_name + ")");
                    args = "pointerOf(" + buffer_name + "), BigInt(" + buffer_name + ".length), " +
                           bitmap_type;
                    break;
                }
            case GenLang::julia:
                {
                    // The Julia idiomatic wrapper for `from_base64` declares the buffer as
                    // `Integer` and narrows it with Cint(), so neither the emitted String nor a
                    // pointer matches it (kwxFFI wrapper defect, filed in kwxFfiIssues.md). The
                    // port's PNG-data factory does take a pointer, and the embedded image is a
                    // base64 text constant, so decode it once and hand the bytes over.
                    FfiUiCall from_base64;
                    if (!ctx.code.FfiResolve("wxBitmap", "from_base64", from_base64))
                    {
                        ctx.ok = false;
                        return {};
                    }
                    const std::string bytes_name = MakeIdentifier(
                        NodeVarName(node, ctx.language) + "_b64_bytes", ctx.language);
                    ctx.writer->writeLine(bytes_name + " = Base64.base64decode(" + b64_name + ")");
                    Code expression(node, ctx.language);
                    expression.Str("wxBitmap_NewFromPNGData(Ptr{Cvoid}(pointer(")
                        .Str(bytes_name)
                        .Str(")), length(")
                        .Str(bytes_name)
                        .Str("))");
                    return AsHandle(ctx.code, expression.GetView());
                }
            case GenLang::luajit:
                {
                    // wxBitmap_CreateFromBase64 takes a void*, and LuaJIT converts a Lua string
                    // only for a `const char*` parameter, so pin a byte view of the base64
                    // constant once and hand that over.
                    const std::string bytes_name = MakeIdentifier(
                        NodeVarName(node, ctx.language) + "_b64_bytes", ctx.language);
                    ctx.writer->writeLine("local " + bytes_name + " = ffi.new(\"uint8_t[?]\", #" +
                                          b64_name + ")");
                    ctx.writer->writeLine("ffi.copy(" + bytes_name + ", " + b64_name + ", #" +
                                          b64_name + ")");
                    args = bytes_name + ", #" + b64_name + ", " + bitmap_type;
                    break;
                }
            case GenLang::rust:
                // `Bitmap::create_from_base64` takes a `*mut c_void` buffer, while `str::as_ptr()`
                // yields `*const u8`; the cast has to name the mutability explicitly.
                args =
                    b64_name + ".as_ptr() as *mut c_void, " + b64_name + ".len(), " + bitmap_type;
                break;
            case GenLang::fortran:
                {
                    // wx_bitmap_create_from_base64 takes a 1-D integer(c_int8_t) array, while
                    // the embedded image is emitted as a character constant. Pin a byte view
                    // of the string once and pass its size.
                    const std::string bytes_name = MakeIdentifier(
                        NodeVarName(node, ctx.language) + "_b64_bytes", ctx.language);
                    ctx.declarations +=
                        "    integer(c_int8_t), dimension(:), allocatable :: " + bytes_name + "\n";
                    ctx.writer->writeLine(bytes_name + " = transfer(" + b64_name + ", " +
                                          bytes_name + ", int(len(" + b64_name +
                                          "), kind=c_size_t))");
                    args = bytes_name + ", size(" + bytes_name + ", kind=c_size_t), " + bitmap_type;
                    break;
                }
            default:
                args = b64_name + ", len(" + b64_name + "), " + bitmap_type;
                break;
        }

        Code expression(node, ctx.language);
        if (!expression.FfiBitmap("from_base64", {}, args))
        {
            ctx.ok = false;
            return {};
        }
        return AsHandle(ctx.code, expression.GetView());
    }

    [[nodiscard]] std::string FontExpr(EmitContext& ctx, Node* node)
    {
        const std::string font_value = node->as_string(prop_font);
        const bool underlined = font_value.contains("underlined");
        // A null face pointer is dereferenced by the kwxFFI font shim, so pass an empty string
        // in the spelling the wrapper expects (a wxString pointer for TypeScript).
        const std::string face = StringArg(ctx.code, "");
        // wxFont's family/style/weight constants carry their full spelling in every wrapper; a
        // bare DEFAULT/NORMAL does not exist.
        std::string args;
        if (ctx.language == GenLang::rust)
        {
            // kwxRust's `Font::new` keeps the wrapper's default style internally, so the
            // constructor has no `style` parameter (see kwxRust/wx/src/font.rs).
            args = "0, " + ctx.code.FfiConstant("FONTFAMILY_DEFAULT") + ", " +
                   ctx.code.FfiConstant("FONTWEIGHT_NORMAL") + ", " +
                   ctx.code.FfiBoolLiteral(underlined) + ", " + face + ", " +
                   ctx.code.FfiConstant("FONTENCODING_DEFAULT");
        }
        else
        {
            args = "0, " + ctx.code.FfiConstant("FONTFAMILY_DEFAULT") + ", " +
                   ctx.code.FfiConstant("FONTSTYLE_NORMAL") + ", " +
                   ctx.code.FfiConstant("FONTWEIGHT_NORMAL") + ", " +
                   ctx.code.FfiBoolLiteral(underlined) + ", " + face + ", " +
                   ctx.code.FfiConstant("FONTENCODING_DEFAULT");
        }

        // The caller owns the assignment prefix; emit the constructor expression only.
        Code statement(node, ctx.language);
        if (!statement.FfiFont({}, args))
        {
            ctx.ok = false;
            return {};
        }
        return std::string(statement.GetView());
    }

    [[nodiscard]] std::string ArgForParam(EmitContext& ctx, Node* node, std::string_view param_name,
                                          bool is_form, const std::string& parent_expr)
    {
        const Code& code = ctx.code;
        if (param_name == "parent")
        {
            return parent_expr;
        }
        if (param_name == "id")
        {
            return code.FfiConstant("ID_ANY");
        }
        if (param_name == "text" || param_name == "title")
        {
            if (is_form)
            {
                return StringArg(code, node->as_string(prop_title));
            }
            return StringArg(code, node->as_string(prop_value));
        }
        if (param_name == "value" || param_name == "label")
        {
            return StringArg(code, node->as_string(prop_value));
        }
        if (param_name == "left" || param_name == "x" || param_name == "top" || param_name == "y")
        {
            const wxPoint position = node->as_wxPoint(prop_pos);
            const int value = (param_name == "left" || param_name == "x") ? position.x : position.y;
            return std::to_string(value);
        }
        if (param_name == "width" || param_name == "height")
        {
            const wxSize size = node->as_wxSize(prop_size);
            const int value = param_name == "width" ? size.x : size.y;
            return std::to_string(value);
        }
        if (param_name == "style")
        {
            std::string styles;
            if (node->HasValue(prop_style))
            {
                styles = RenderConstants(code, node->as_string(prop_style));
            }
            if (!styles.empty())
            {
                return styles;
            }
            const std::string default_style_name =
                DefaultStyleName(std::string(node->get_DeclName()));
            // A class without a default style takes a literal 0 -- spelling an empty constant
            // name would emit `wx.`.
            return default_style_name.empty() ? "0" : code.FfiConstant(default_style_name);
        }
        if (param_name == "orient")
        {
            return RenderConstants(code, node->as_string(prop_orientation));
        }
        if (param_name == "flags")
        {
            return node->HasValue(prop_flags) ? RenderConstants(code, node->as_string(prop_flags)) :
                                                "0";
        }
        if (param_name == "bitmap")
        {
            return BitmapExpr(ctx, node);
        }
        if (param_name == "font")
        {
            return "font";
        }
        if (param_name == "sizer" || param_name == "window")
        {
            return parent_expr;
        }
        if (param_name == "update_ui")
        {
            return code.FfiBoolLiteral(false);
        }
        return "0";
    }

    [[nodiscard]] std::string CtorArgs(EmitContext& ctx, Node* node, const FfiUiCall& ctor,
                                       bool is_form, const std::string& parent_expr)
    {
        std::string args;
        for (const FfiUiParam& param: ctor.params)
        {
            const std::string value = ctx.code.FfiCoerceParam(
                param.type, ArgForParam(ctx, node, param.name, is_form, parent_expr));
            if (value.empty())
            {
                continue;
            }
            if (!args.empty())
            {
                args += ", ";
            }
            args += value;
        }
        return args;
    }

    void WriteStatement(EmitContext& ctx, Code& statement, const std::string& node_class,
                        const std::string& var_name, std::string_view property,
                        std::string_view role)
    {
        if (statement.empty())
        {
            return;
        }

        const std::string_view text = statement.GetView();
        size_t start = 0;
        while (start <= text.size())
        {
            const size_t end = text.find('\n', start);
            const std::string_view line = text.substr(
                start, end == std::string_view::npos ? std::string_view::npos : end - start);
            if (!line.empty())
            {
                // Rust is the only FFI target whose statements are terminated explicitly. A line
                // the Code writer wrapped mid-statement ends with an operator, so terminating it
                // would split the assignment; the continuation line carries the `;` instead.
                if (ctx.language == GenLang::rust && line.back() != ';' && line.back() != '{' &&
                    line.back() != '}' && line.back() != '=' && line.back() != ',' &&
                    line.back() != '(' && line.back() != '.' && line.back() != '|' &&
                    line.back() != '&' && line.back() != '+' && !line.starts_with("//"))
                {
                    ctx.writer->writeLine(std::string(line) + ';');
                }
                else
                {
                    ctx.writer->writeLine(line);
                }
                FfiSourceMapEntry entry;
                entry.node_class = node_class;
                entry.var_name = var_name;
                entry.property = std::string(property);
                entry.role = std::string(role);
                SourceMapStorage().emplace_back(std::move(entry));
            }
            if (end == std::string_view::npos)
            {
                break;
            }
            start = end + 1;
        }
    }

    void BeginAssign(Code& statement, GenLang language, const std::string& name, bool is_decl)
    {
        if (!is_decl)
        {
            statement.Str(name).Str(" = ");
            return;
        }

        switch (language)
        {
            case GenLang::go:
                statement.Str(name).Str(" := ");
                break;
            case GenLang::typescript:
                statement.Str("const ").Str(name).Str(" = ");
                break;
            case GenLang::luajit:
                statement.Str("local ").Str(name).Str(" = ");
                break;
            case GenLang::rust:
                statement.Str("let ").Str(name).Str(" = ");
                break;
            default:
                statement.Str(name).Str(" = ");
                break;
        }
    }

    void EmitFormCreation(EmitContext& ctx, const std::string& parent_expr)
    {
        Node* form = ctx.form;
        const std::string wx_class(form->get_DeclName());
        FfiUiCall ctor;
        if (!ctx.code.FfiResolve(wx_class, "ctor", ctor))
        {
            ctx.ok = false;
            return;
        }

        const std::string args = CtorArgs(ctx, form, ctor, true, parent_expr);
        Code statement(form, ctx.language);
        if (ctx.language == GenLang::go)
        {
            if (!statement.FfiCtor(wx_class, args, ctx.form_receiver))
            {
                ctx.ok = false;
                return;
            }
        }
        else
        {
            BeginAssign(statement, ctx.language, ctx.form_receiver, true);
            if (!statement.FfiCtor(wx_class, args))
            {
                ctx.ok = false;
                return;
            }
        }
        WriteStatement(ctx, statement, wx_class, ctx.form_receiver, "ctor", "ctor");
    }

    [[nodiscard]] std::string PointerRef(GenLang language, Node* node, const std::string& name)
    {
        const std::string wx_class(node->get_DeclName());
        if (language == GenLang::rust)
        {
            // kwxRust passes the wrapper object itself (`&impl WxWindow`, `&impl WxSizer`);
            // `.as_ptr()` would hand a raw handle to a parameter that takes the object.
            return name;
        }
        if (HasRole(language, wx_class, "ptr"))
        {
            Code expression(node, language);
            if (expression.FfiPtr(wx_class, name))
            {
                return std::string(expression.GetView());
            }
        }
        return name;
    }

    // The expression that turns a wrapper object back into the handle the wrapper functions
    // take. A language whose descriptor records no pointer accessor passes the object through.
    [[nodiscard]] std::string AsHandle(const Code& code, std::string_view expression)
    {
        if (expression.empty())
        {
            return std::string(expression);
        }
        const std::string accessor = code.FfiPointerAccessor();
        if (!accessor.empty())
        {
            return std::string(expression) + accessor;
        }
        if (code.get_language() == GenLang::typescript)
        {
            // The TypeScript wrapper exposes the native pointer as the read-only `ptr` getter.
            return std::string(expression) + ".ptr";
        }
        return std::string(expression);
    }

    // The kwxRust wrapper's methods take the wrapper object (`&impl WxXxx`), while the
    // pointer-oriented ports take the `.Ptr()`/`.as_ptr()` handle. A Rust argument that is a
    // wrapper object therefore needs the reference its signature declares.
    [[nodiscard]] std::string MethodArg(const Code& code, const std::string& expression)
    {
        if (code.get_language() == GenLang::rust && !expression.empty() &&
            expression.front() != '&')
        {
            return "&" + expression;
        }
        return expression;
    }

    struct SizerItemFlags
    {
        int proportion { 0 };
        bool expand { false };
        std::vector<std::string> sides;
        int border { 0 };
    };

    // Reads an item's proportion, wxEXPAND bit and border sides from the node properties.
    [[nodiscard]] SizerItemFlags ReadSizerItemFlags(const Code& code, Node* node)
    {
        SizerItemFlags item;
        item.proportion = node->as_int(prop_proportion);

        const std::string flags =
            node->HasValue(prop_flags) ? std::string(node->as_string(prop_flags)) : std::string();
        size_t start = 0;
        while (start <= flags.size())
        {
            const size_t end = flags.find('|', start);
            const std::string token = Trim(
                flags.substr(start, end == std::string::npos ? std::string::npos : end - start));
            if (token == "wxEXPAND")
            {
                item.expand = true;
            }
            else if (token == "wxALL" || token == "wxLEFT" || token == "wxRIGHT" ||
                     token == "wxTOP" || token == "wxBOTTOM")
            {
                item.sides.emplace_back(code.FfiConstant(std::string_view(token).substr(2)));
            }
            if (end == std::string::npos)
            {
                break;
            }
            start = end + 1;
        }

        if (!item.sides.empty())
        {
            item.border = node->HasValue(prop_border) ? node->as_int(prop_border) : 5;
            if (item.border <= 0)
            {
                item.border = 5;
            }
        }
        return item;
    }

    // True when the descriptor's box-sizer add callable takes a built wxSizerFlags handle
    // (`flags`, Go's AddWindowFlags/AddSizerFlags) rather than a numeric mask (`flag`).
    [[nodiscard]] bool SizerItemTakesFlags(Code& code, bool item_is_sizer)
    {
        FfiUiCall call;
        if (!code.FfiResolve("wxBoxSizer", item_is_sizer ? "add_sizer" : "add_window", call))
        {
            return false;
        }
        for (const FfiUiParam& param: call.params)
        {
            if (param.name == "flags")
            {
                return true;
            }
        }
        return false;
    }

    // Emits one sizer item: the descriptor's flags chain when the add callable takes one, then
    // the add call itself with every argument taken from the descriptor's parameter names.
    void EmitSizerItem(EmitContext& ctx, const std::string& sizer_name, Node* item_node,
                       const std::string& item_expr, bool item_is_sizer)
    {
        const std::string item_var = NodeVarName(item_node, ctx.language);
        const SizerItemFlags item = ReadSizerItemFlags(ctx.code, item_node);

        std::string flags_ptr;
        if (SizerItemTakesFlags(ctx.code, item_is_sizer))
        {
            const std::string flags_name = MakeIdentifier(item_var + "_flags", ctx.language);
            Code flags_statement(item_node, ctx.language);
            if (!flags_statement.FfiFlags(flags_name, item.proportion, item.expand,
                                          CombineFlags(ctx.language, item.sides)))
            {
                ctx.ok = false;
                return;
            }
            WriteStatement(ctx, flags_statement, std::string(item_node->get_DeclName()), flags_name,
                           "flags", "flags");
            flags_ptr = AsHandle(ctx.code, flags_name);
        }

        // The item mask is the wrapper's numeric flag argument. `wxEXPAND` is a mask bit for
        // a numeric-flag add callable, so it belongs in the expression; the Go-shaped
        // wxSizerFlags path carries it through FfiFlags() instead.
        std::vector<std::string> flag_parts;
        if (item.expand)
        {
            flag_parts.emplace_back(ctx.code.FfiConstant("EXPAND"));
        }
        flag_parts.insert(flag_parts.end(), item.sides.begin(), item.sides.end());
        std::string flag_expr = CombineFlags(ctx.language, flag_parts);
        if (flag_expr.empty())
        {
            flag_expr = "0";
        }

        Code add_statement(item_node, ctx.language);
        if (!add_statement.FfiSizerItem(sizer_name, item_expr, item_is_sizer, item.proportion,
                                        flag_expr, item.border, flags_ptr))
        {
            ctx.ok = false;
            return;
        }
        WriteStatement(ctx, add_statement, "wxBoxSizer", sizer_name,
                       item_is_sizer ? "add_sizer" : "add_window",
                       item_is_sizer ? "add_sizer" : "add_window");
    }

    void EmitWidget(EmitContext& ctx, Node* node, const std::string& parent_expr, bool in_sizer);

    // The stock-button mask the C1 `std_buttons.create` call takes, built from the node's button
    // properties exactly as gen_std_dlgbtn_sizer.cpp's BuildDialogButtonFlags does.
    [[nodiscard]] std::string StdButtonFlags(const Code& code, Node* node)
    {
        std::vector<std::string> parts;
        const auto add_flag = [&parts, &code](std::string_view name)
        {
            parts.emplace_back(code.FfiConstant(name));
        };

        if (node->as_bool(prop_OK))
        {
            add_flag("OK");
        }
        else if (node->as_bool(prop_Yes))
        {
            add_flag("YES");
        }
        if (node->as_bool(prop_No))
        {
            add_flag("NO");
        }
        if (node->as_bool(prop_Cancel))
        {
            add_flag("CANCEL");
        }
        else if (node->as_bool(prop_Close))
        {
            add_flag("CLOSE");
        }
        if (node->as_bool(prop_Apply))
        {
            add_flag("APPLY");
        }
        if (node->as_bool(prop_Help))
        {
            add_flag("HELP");
        }
        return CombineFlags(code.get_language(), parts);
    }

    // `create` is a method on the dialog wrapper, so its receiver is the form object -- not the
    // form's native handle, which is only what a child constructor takes as its parent.
    void EmitStdButtons(EmitContext& ctx, Node* node, const std::string& sizer_name)
    {
        const std::string wx_class(node->get_DeclName());
        const std::string name = NodeVarName(node, ctx.language);
        const std::string flags = StdButtonFlags(ctx.code, node);
        if (flags.empty())
        {
            return;  // the form requests no stock buttons
        }

        if (ctx.language == GenLang::fortran)
        {
            ctx.declarations +=
                "    type(" + GetFortranLocalType(ctx.language, wx_class) + ") :: " + name + "\n";
        }

        Code statement(node, ctx.language);
        BeginAssign(statement, ctx.language, name, true);
        if (!statement.FfiStockButton(ctx.form_receiver, name, flags, true))
        {
            ctx.ok = false;
            return;
        }
        WriteStatement(ctx, statement, wx_class, name, "buttons", "create");

        // A create that runs on the parent (Go, LuaJIT, TypeScript) adds the buttons and
        // realizes them inside the wrapper -- wxDialog::CreateStdDialogButtonSizer ends with
        // Realize(). A constructor-shaped create still needs the explicit Realize() step.
        FfiUiCall create_call;
        const bool parent_owned = ctx.code.FfiResolve(wx_class, "create", create_call) &&
                                  create_call.idiom == "method-on-parent";
        // Julia's create runs on the dialog too (see Code::FfiStockButton); the wrapper's
        // CreateStdDialogButtonSizer ends with Realize(), so no separate step is emitted.
        const bool realized_by_create =
            parent_owned || ((ctx.language == GenLang::julia || ctx.language == GenLang::rust) &&
                             create_call.params.empty());
        if (!realized_by_create)
        {
            EmitMethodStatement(ctx, node, wx_class, "realize", name, {}, wx_class, name);
        }

        if (!sizer_name.empty())
        {
            // A method-on-parent create returns the wrapper object in every port except Go,
            // where it returns the raw handle already.
            const std::string item = (parent_owned && ctx.language != GenLang::typescript) ?
                                         name :
                                         PointerRef(ctx.language, node, name);
            EmitSizerItem(ctx, sizer_name, node, item, true);
        }
    }

    void EmitSizer(EmitContext& ctx, Node* sizer_node, const std::string& parent_expr)
    {
        const std::string wx_class = "wxBoxSizer";
        const std::string name = NodeVarName(sizer_node, ctx.language);

        if (ctx.language == GenLang::fortran)
        {
            ctx.declarations +=
                "    type(" + GetFortranLocalType(ctx.language, wx_class) + ") :: " + name + "\n";
        }

        Code statement(sizer_node, ctx.language);
        BeginAssign(statement, ctx.language, name, true);
        const std::string orient =
            RenderConstants(ctx.code, sizer_node->as_string(prop_orientation));
        if (!statement.FfiCtor(wx_class, orient.empty() ? "0" : orient))
        {
            ctx.ok = false;
            return;
        }
        WriteStatement(ctx, statement, wx_class, name, "orientation", "ctor");

        for (const auto& child: sizer_node->get_ChildNodePtrs())
        {
            if (child->is_Gen(gen_wxContextMenuEvent))
            {
                continue;
            }

            if (child->is_Sizer())
            {
                EmitSizer(ctx, child.get(), parent_expr);
                const std::string child_name = NodeVarName(child.get(), ctx.language);
                EmitSizerItem(ctx, name, child.get(),
                              PointerRef(ctx.language, child.get(), child_name), true);
                continue;
            }

            if (child->is_Gen(gen_wxStdDialogButtonSizer))
            {
                EmitStdButtons(ctx, child.get(), name);
                continue;
            }

            EmitWidget(ctx, child.get(), parent_expr, true);
            const std::string child_name = NodeVarName(child.get(), ctx.language);
            EmitSizerItem(ctx, name, child.get(), PointerRef(ctx.language, child.get(), child_name),
                          false);
        }
    }

    void EmitWidget(EmitContext& ctx, Node* node, const std::string& parent_expr, bool in_sizer)
    {
        const std::string wx_class(node->get_DeclName());
        const std::string name = NodeVarName(node, ctx.language);

        const bool has_value = node->HasValue(prop_value);
        const bool has_font = node->HasValue(prop_font);
        // A widget nothing refers to again must not be bound to a variable in Go: an unused
        // local is a compile error, while a bare call statement is legal.
        const bool needs_variable =
            in_sizer || has_value || has_font || ctx.language != GenLang::go;

        if (ctx.language == GenLang::fortran && needs_variable)
        {
            ctx.declarations +=
                "    type(" + GetFortranLocalType(ctx.language, wx_class) + ") :: " + name + "\n";
        }

        FfiUiCall ctor;
        if (!ctx.code.FfiResolve(wx_class, "ctor", ctor))
        {
            ctx.ok = false;
            return;
        }

        const std::string args = CtorArgs(ctx, node, ctor, false, parent_expr);
        Code statement(node, ctx.language);
        if (needs_variable)
        {
            BeginAssign(statement, ctx.language, name, true);
        }
        else
        {
            // Go: the control is created for its side effect only, so say so explicitly.
            statement.Str("_ = ");
        }
        if (!statement.FfiCtor(wx_class, args))
        {
            ctx.ok = false;
            return;
        }
        WriteStatement(ctx, statement, wx_class, name, "ctor", "ctor");

        FfiUiCall set_value_call;
        if (has_value && ctx.code.FfiResolve(wx_class, "set_value", set_value_call))
        {
            EmitMethodStatement(ctx, node, wx_class, "set_value", name,
                                StringArg(ctx.code, node->as_string(prop_value)), wx_class, name);
        }

        FfiUiCall set_font_call;
        if (has_font && ctx.code.FfiResolve(wx_class, "set_font", set_font_call))
        {
            const std::string font_name = MakeIdentifier(name + "_font", ctx.language);
            if (ctx.language == GenLang::fortran)
            {
                ctx.declarations += "    type(" + GetFortranLocalType(ctx.language, "wxFont") +
                                    ") :: " + font_name + "\n";
            }
            const std::string font_expression = FontExpr(ctx, node);
            if (!font_expression.empty())
            {
                Code font(node, ctx.language);
                BeginAssign(font, ctx.language, font_name, true);
                font.Str(font_expression);
                WriteStatement(ctx, font, "wxFont", font_name, "font", "create");

                EmitMethodStatement(ctx, node, wx_class, "set_font", name,
                                    MethodArg(ctx.code, AsHandle(ctx.code, font_name)), wx_class,
                                    name);
            }
        }
    }

    [[nodiscard]] std::string HandlerName(const FfiFormInfo& info, GenLang language)
    {
        if (IsSnakeLanguage(language))
        {
            return "on_" + info.snake + "_close";
        }
        return "on" + info.pascal_name + "Close";
    }

    void EmitCloseEvent(EmitContext& ctx)
    {
        const NodeEvent* close_event = ctx.form->get_Event("wxEVT_CLOSE_WINDOW");
        if (!close_event)
        {
            return;
        }

        const std::string handler = HandlerName(ctx.info, ctx.language);

        if (ctx.language == GenLang::fortran)
        {
            // A Fortran bind needs a wxclosure_t built from a bind(C) procedure, and the
            // generated form owns no handler. wxFrame and wxDialog already close on
            // wxEVT_CLOSE_WINDOW, which is exactly what quick_main.f90 relies on.
            ctx.handlers += "! " + handler + ": wxEVT_CLOSE_WINDOW is handled by quick_main.f90\n";
            return;
        }

        if (ctx.language == GenLang::rust)
        {
            // kwxRust's EvtHandler::bind takes a wxClosure built from a C callback of the shape
            // `void (*)(void* closure, void* data, void* event)` (kwxFFI's ClosureFun), so the
            // generated form builds one Closure and binds it. The wrapper's Event::from_raw is
            // pub(crate), so the handler skips through the generated raw binding layer instead:
            // Skip(true) hands the event back to wxWidgets, which runs its default close handling
            // (Destroy for a frame, EndModal for a dialog) -- the shape Go, Julia and Lua use.
            const std::string closure_name = MakeIdentifier("close_closure", ctx.language);
            Code closure(ctx.form, ctx.language);
            closure.Str("let ")
                .Str(closure_name)
                .Str(" = Closure::new(")
                .Str(handler)
                .Str(" as *mut c_void, std::ptr::null_mut())");
            WriteStatement(ctx, closure, std::string(ctx.form->get_DeclName()), closure_name,
                           "wxEVT_CLOSE_WINDOW", "bind");

            const std::string rust_args =
                ctx.code.FfiConstant("ID_ANY") + ", " + ctx.code.FfiConstant("ID_ANY") + ", " +
                ctx.code.FfiConstant("EVT_CLOSE_WINDOW") + ", &" + closure_name;
            Code bind_statement(ctx.form, ctx.language);
            if (!bind_statement.FfiBind(ctx.form->get_DeclName(), ctx.form_receiver, rust_args))
            {
                return;
            }
            WriteStatement(ctx, bind_statement, std::string(ctx.form->get_DeclName()),
                           ctx.form_receiver, "wxEVT_CLOSE_WINDOW", "bind");

            ctx.handlers +=
                "extern \"C\" fn " + handler +
                "(_closure: *mut c_void, _data: *mut c_void, event: *mut c_void) {\n"
                "    // wxEVT_CLOSE_WINDOW: Skip(true) so wxWidgets runs its default close\n"
                "    // handling (Destroy for a frame, EndModal for a dialog).\n"
                "    if !event.is_null() {\n"
                "        unsafe { wx::sys::wxEvent_Skip(event, 1); }\n"
                "    }\n"
                "}\n";
            return;
        }

        if (ctx.language == GenLang::typescript)
        {
            // TypeScript's Bind takes a native closure pointer, not a JavaScript function: build
            // a Callback trampoline, wrap it with createWxClosure, then bind that pointer.
            const std::string callback_name = MakeIdentifier("close_callback", ctx.language);
            const std::string closure_name = MakeIdentifier("close_closure", ctx.language);

            Code callback(ctx.form, ctx.language);
            callback.Str("const ")
                .Str(callback_name)
                .Str(" = new Callback({ parameters: [\"pointer\", \"pointer\", \"pointer\"], "
                     "result: \"void\" }, ")
                .Str(handler)
                .Str(")");
            WriteStatement(ctx, callback, std::string(ctx.form->get_DeclName()), callback_name,
                           "wxEVT_CLOSE_WINDOW", "bind");

            Code closure(ctx.form, ctx.language);
            closure.Str("const ")
                .Str(closure_name)
                .Str(" = createWxClosure(")
                .Str(callback_name)
                .Str(".pointer, null)!");
            WriteStatement(ctx, closure, std::string(ctx.form->get_DeclName()), closure_name,
                           "wxEVT_CLOSE_WINDOW", "bind");

            const std::string ts_args =
                ctx.code.FfiConstant("ID_ANY") + ", " + ctx.code.FfiConstant("ID_ANY") + ", " +
                ctx.code.FfiConstant("EVT_CLOSE_WINDOW") + ", " + closure_name + ".ptr";
            Code ts_bind(ctx.form, ctx.language);
            if (!ts_bind.FfiBind(ctx.form->get_DeclName(), ctx.form_receiver, ts_args))
            {
                return;
            }
            WriteStatement(ctx, ts_bind, std::string(ctx.form->get_DeclName()), ctx.form_receiver,
                           "wxEVT_CLOSE_WINDOW", "bind");

            // Closing the form ends the application, exactly as the trusted samples do
            // (samples/combo, samples/animate). The runner still owns shutdown/reporting.
            ctx.handlers += "function " + handler +
                            "(_closureFun: Pointer, _data: Pointer, _eventPtr: Pointer): void {\n"
                            "    exitMainLoop();\n"
                            "}\n";
            return;
        }

        if (ctx.language == GenLang::julia)
        {
            // The Julia wrapper binds through the hand-written wxClosure bridge in
            // kwxJulia/src/core/kwxapp.jl (`wx_connect!`); Base.bind itself needs a wxClosure,
            // which a generated form cannot build. The handler skips the event so wxWidgets
            // still runs its default close handling (Destroy for a frame, EndModal for the
            // dialog) -- the shape the Go slice settled on.
            Code bind_statement(ctx.form, ctx.language);
            bind_statement.Str("wx_connect!(")
                .Str(ctx.form_receiver)
                .Str(", ")
                .Str(ctx.code.FfiConstant("EVT_CLOSE_WINDOW"))
                .Str(", ")
                .Str(handler)
                .Str(")");
            WriteStatement(ctx, bind_statement, std::string(ctx.form->get_DeclName()),
                           ctx.form_receiver, "wxEVT_CLOSE_WINDOW", "bind");
            // One-line definition: the writer strips leading whitespace from a replayed line, so
            // a multi-line body would lose its indentation; every other FFI close handler is a
            // one-liner for the same reason.
            ctx.handlers += handler + "(event) = skip!(event, true)\n";
            return;
        }

        if (ctx.language == GenLang::luajit)
        {
            // kwxLuaJIT's wx module replaces wxEvtHandler:Connect(first, last, type, closure)
            // with the callback bridge in lua/wx/events.lua -- Connect(event_type, handler, id) --
            // and aliases Bind to it, so a generated form binds through that two-argument form
            // exactly as the samples do. The descriptor's `bind` role records the raw
            // Connect(first, last, type, closure) layer underneath, which a generated form
            // cannot build (it needs a wxClosure).
            Code bind_statement(ctx.form, ctx.language);
            bind_statement.Str(ctx.form_receiver)
                .Str(":Connect(")
                .Str(ctx.code.FfiConstant("EVT_CLOSE_WINDOW"))
                .Str(", ")
                .Str(handler)
                .Str(")");
            WriteStatement(ctx, bind_statement, std::string(ctx.form->get_DeclName()),
                           ctx.form_receiver, "wxEVT_CLOSE_WINDOW", "bind");

            // The Lua bridge returns a handled event, so a handler that does nothing would veto
            // the close. Skip(true) hands the event back to wxWidgets, which runs its default
            // close handling (Destroy for a frame, EndModal for the dialog) -- the same shape the
            // Go and Julia handlers take.
            ctx.handlers += "local function " + handler +
                            "(event)\n"
                            "    if event ~= nil then\n"
                            "        wx_classes.wxCloseEvent.FromPtr(event):Skip(true)\n"
                            "    end\n"
                            "end\n";
            return;
        }

        std::string args;
        switch (ctx.language)
        {
            case GenLang::go:
                // kwxGO's wx.EvtHandler.Bind(evtType, handler): the event type and a Go closure
                // of type wx.EventHandler. The generated handler calls Skip so wxWidgets still
                // runs its default close handling (Destroy for a frame, EndModal for a dialog).
                args = ctx.code.FfiConstant("EVT_CLOSE_WINDOW") + ", " + handler;
                break;
            default:
                args = ctx.code.FfiConstant("ID_ANY") + ", " + ctx.code.FfiConstant("ID_ANY") +
                       ", " + ctx.code.FfiConstant("EVT_CLOSE") + ", " + ctx.code.FfiNullLiteral();
                break;
                break;
        }

        Code bind_statement(ctx.form, ctx.language);
        if (!bind_statement.FfiBind(ctx.form->get_DeclName(), ctx.form_receiver, args))
        {
            return;  // a language without an event surface simply emits nothing
        }
        WriteStatement(ctx, bind_statement, std::string(ctx.form->get_DeclName()),
                       ctx.form_receiver, "wxEVT_CLOSE_WINDOW", "bind");

        switch (ctx.language)
        {
            case GenLang::go:
                ctx.handlers += "func " + handler + "(evt *wx.Event) {\n\tevt.Skip(true)\n}\n";
                break;
            default:
                ctx.handlers +=
                    "! " + handler + ": wxEVT_CLOSE_WINDOW is handled by quick_main.f90\n";
                break;
        }
    }
}  // namespace ffi_emit_detail

using namespace ffi_emit_detail;
std::vector<std::string> GetFortranUseModules(GenLang language)
{
    std::vector<std::string> modules { "kwx_types", "kwxffi" };

    if (const FfiUiDescriptor* descriptor = GetFfiUiDescriptor(language); descriptor != nullptr)
    {
        for (const std::pair<const std::string, FfiUiClass>& class_entry: descriptor->classes)
        {
            modules.emplace_back(MakeIdentifier(class_entry.first, language));
            for (const std::pair<const std::string, FfiUiCall>& role_entry:
                 class_entry.second.roles)
            {
                if (!role_entry.second.owner.empty())
                {
                    modules.emplace_back(MakeIdentifier(role_entry.second.owner, language));
                }
            }
        }
        for (const std::string_view event_key:
             { std::string_view("bind"), std::string_view("bind_id") })
        {
            const FfiUiCallMap::const_iterator event_iter = descriptor->events.find(event_key);
            if (event_iter != descriptor->events.end() && !event_iter->second.owner.empty())
            {
                modules.emplace_back(MakeIdentifier(event_iter->second.owner, language));
            }
        }
    }

    // The font and bitmap blocks name module procedures whose classes are not part of the
    // required-class list; their modules still have to be imported.
    modules.emplace_back(MakeIdentifier("wxFont", language));
    modules.emplace_back(MakeIdentifier("wxBitmap", language));

    std::sort(modules.begin(), modules.end());
    modules.erase(std::unique(modules.begin(), modules.end()), modules.end());
    return modules;
}

FfiFormInfo GetFfiFormInfo(Node* form, GenLang language)
{
    std::string source;
    if (form && Project.get_ProjectNode())
    {
        auto [path, has_base_file] = Project.GetOutputPath(form, language);
        if (has_base_file)
        {
            const std::string full(path.c_str());
            const size_t slash = full.find_last_of("/\\");
            source = slash == std::string::npos ? full : full.substr(slash + 1);
            const size_t dot_position = source.rfind('.');
            if (dot_position != std::string::npos)
            {
                source.erase(dot_position);
            }
        }
    }
    if (source.empty() && form)
    {
        source = form->as_string(prop_class_name);
    }

    FfiFormInfo info;
    info.basename = source;
    const std::vector<std::string> tokens = SplitTokens(source);
    info.snake = JoinSnake(tokens);
    info.pascal_name = JoinPascal(tokens);
    info.camel = info.pascal_name;
    if (!info.camel.empty())
    {
        info.camel[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(info.camel[0])));
    }
    return info;
}

bool EmitFfiFormBody(Code& code, WriteCode* writer, Node* form, const FfiFormInfo& info,
                     const std::string& form_receiver, std::string& declarations,
                     std::string& handlers, const std::string& parent_expr)
{
    if (!form || !writer)
    {
        return false;
    }

    SourceMapStorage().clear();

    EmitContext ctx { code,          writer,        form,          info,  code.get_language(),
                      form_receiver, std::string(), std::string(), false, false,
                      true };
    ctx.declarations.clear();
    ctx.handlers.clear();

    EmitFormCreation(ctx, parent_expr);

    // Children are parented to the form's native handle, not to the wrapper object.
    const std::string parent_ptr = AsHandle(ctx.code, form_receiver);

    for (const auto& child: form->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_wxContextMenuEvent))
        {
            continue;
        }
        if (child->is_Sizer())
        {
            EmitSizer(ctx, child.get(), parent_ptr);
            continue;
        }
        if (child->is_Gen(gen_wxStdDialogButtonSizer))
        {
            EmitStdButtons(ctx, child.get(), {});
            continue;
        }
        EmitWidget(ctx, child.get(), parent_ptr, false);
    }

    // Attach the form's first sizer
    if (form->get_ChildCount() && form->get_Child(0)->is_Sizer())
    {
        const std::string sizer_name = NodeVarName(form->get_Child(0), ctx.language);
        const std::string sizer_ref =
            MethodArg(ctx.code, PointerRef(ctx.language, form->get_Child(0), sizer_name));
        const std::string args = sizer_ref + ", " + ctx.code.FfiBoolLiteral(true);
        EmitMethodStatement(ctx, form, std::string(form->get_DeclName()), "set_sizer",
                            form_receiver, args, std::string(form->get_DeclName()), form_receiver);
    }

    // Form-level minimum size and centring, both descriptor roles.
    const wxSize min_size = form->as_wxSize(prop_minimum_size);
    FfiUiCall min_size_call;
    if (min_size != wxDefaultSize &&
        ctx.code.FfiResolve(form->get_DeclName(), "set_min_size", min_size_call))
    {
        std::string args = std::to_string(min_size.x) + ", " + std::to_string(min_size.y);
        if (ctx.language == GenLang::rust)
        {
            // kwxRust's set_min_size takes the wrapper's Size struct, not two integers.
            args = "wx::support::Size { width: " + std::to_string(min_size.x) +
                   ", height: " + std::to_string(min_size.y) + " }";
        }
        EmitMethodStatement(ctx, form, std::string(form->get_DeclName()), "set_min_size",
                            form_receiver, args, std::string(form->get_DeclName()), form_receiver);
    }

    FfiUiCall center_call;
    if (form->HasValue(prop_center) && !form->as_string(prop_center).empty() &&
        !form->is_PropValue(prop_center, "no") &&
        ctx.code.FfiResolve(form->get_DeclName(), "center", center_call))
    {
        EmitMethodStatement(ctx, form, std::string(form->get_DeclName()), "center", form_receiver,
                            RenderConstants(ctx.code, form->as_string(prop_center)),
                            std::string(form->get_DeclName()), form_receiver);
    }

    EmitCloseEvent(ctx);

    declarations = ctx.declarations;
    handlers = ctx.handlers;
    return ctx.ok;
}

const std::vector<FfiSourceMapEntry>& GetFfiSourceMapEntries()
{
    return SourceMapStorage();
}

void ResetFfiSourceMap()
{
    SourceMapStorage().clear();
}

void WriteFfiSourceMap(const std::string& output_path)
{
    wxString enabled;
    if (!wxGetEnv("WXUE_FFI_SOURCEMAP", &enabled) || enabled.empty() || enabled == "0")
    {
        return;
    }

    const std::string map_path = output_path + ".map.json";
    wxFile file;
    if (!file.Create(map_path, true))
    {
        return;
    }

    std::string json = "{\n  \"schema\": 1,\n  \"entries\": [\n";
    bool is_first = true;
    for (const FfiSourceMapEntry& entry: GetFfiSourceMapEntries())
    {
        if (!is_first)
        {
            json += ",\n";
        }
        is_first = false;
        json += "    { \"line\": " + std::to_string(entry.line) + ", \"node\": \"" +
                entry.node_class + "\", \"name\": \"" + entry.var_name + "\", \"property\": \"" +
                entry.property + "\", \"role\": \"" + entry.role + "\" }";
    }
    json += "\n  ]\n}\n";
    file.Write(json);
}
