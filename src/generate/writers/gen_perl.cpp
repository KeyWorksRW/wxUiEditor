/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate Perl code files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <set>
#include <thread>

#include <frozen/map.h>

#include "gen_perl.h"
#include "gen_script_common.h"  // Common functions for generating Script Languages

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "code.h"             // Code -- Helper class for generating code
#include "comment_blocks.h"   // Shared generated comment blocks
#include "common_strings.h"   // Common strings used in code generation
#include "file_codewriter.h"  // FileCodeWriter -- Classs to write code to disk
#include "gen_common.h"       // Common component functions
#include "gen_timer.h"        // TimerGenerator class
#include "image_gen.h"        // Functions for generating embedded images
#include "image_handler.h"    // ImageHandler class
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "tt_view_vector.h"   // tt_view_vector -- Read/Write line-oriented strings/files
#include "utils.h"            // Miscellaneous utilities
#include "write_code.h"       // Write code to Scintilla or file

#include "../customprops/eventhandler_dlg.h"  // EventHandlerDlg static functions

using namespace code;
using namespace GenEnum;

// clang-format off

inline constexpr const auto txt_perl_frame_app =
R"===(# Sample code for displaying your MainFrame window.
# Place this code after the closing comment block in your
# generated file. You can then call 'Perl filename.pl'
# where filename.pl is the name of your generated file.

package main;

my $app = Wx::SimpleApp->new;
my $frame = MainFrame->new(undef, -1, "wxPerl app");
$frame->Show;
$app->MainLoop;
)===";

inline constexpr const auto txt_perl_get_bundle =
R"===(
# Loads image(s) from a string and returns a Wx::BitmapBundle object.
sub wxue_get_bundle {
    my ($image_data1, $image_data2, $image_data3) = @_;

    my $image1 = Wx::Image->new;
    $image1->LoadStream(IO::String->new($image_data1));

    if (defined $image_data2) {
        my $image2 = Wx::Image->new;
        $image2->LoadStream(IO::String->new($image_data2));
        if (defined $image_data3) {
            my $image3 = Wx::Image->new;
            $image3->LoadStream(IO::String->new($image_data3));
            my @bitmaps = (
                Wx::Bitmap->new($image1),
                Wx::Bitmap->new($image2),
                Wx::Bitmap->new($image3)
            );
            my $bundle = Wx::BitmapBundle::FromBitmaps(\@bitmaps);
            return $bundle;
        } else {
            my $bundle = Wx::BitmapBundle::FromBitmaps(
                Wx::Bitmap->new($image1),
                Wx::Bitmap->new($image2)
            );
            return $bundle;
        }
    }
    my $bundle = Wx::BitmapBundle::FromImage($image1);
    return $bundle;
})===";

// clang-format on

// extern constexpr auto map_perl_constants = frozen::make_map<GenEnum::PropName, std::string_view>;

// If the node contains the specified property, then the string contains all
// possible contants that could be used separated by spaces.
constexpr auto map_perl_constants = frozen::make_map<GenEnum::PropName, std::string_view>({

    { prop_bitmap, "wxNullBitmap" },

});

PerlCodeGenerator::PerlCodeGenerator(Node* form_node) : BaseCodeGenerator(GEN_LANG_PERL, form_node)
{
}

auto PerlCodeGenerator::InitializeThreads(std::set<std::string>& img_include_set)
    -> std::tuple<std::thread, std::thread, std::thread>
{
    auto thrd_get_events = std::thread(&PerlCodeGenerator::CollectEventHandlers, this, m_form_node,
                                       std::ref(m_events));
    auto thrd_need_img_func =
        std::thread(&PerlCodeGenerator::ParseImageProperties, this, m_form_node);
    auto thrd_collect_img_headers = std::thread(&PerlCodeGenerator::CollectImageHeaders, this,
                                                m_form_node, std::ref(img_include_set));

    return { std::move(thrd_get_events), std::move(thrd_need_img_func),
             std::move(thrd_collect_img_headers) };
}

auto PerlCodeGenerator::WriteSourceHeader() -> void
{
    m_source->SetLastLineBlank();

#if !defined(_DEBUG)
    if (m_panel_type == NOT_PANEL)
#else
    if (m_panel_type != NOT_PANEL)
    {
        m_source->writeLine("# The following comment block is only displayed in a _DEBUG build, "
                            "or when written to a file.\n\n");
    }
#endif  // _DEBUG
    {
        m_source->writeLine(txt_PoundCmtBlock);

        if (Project.HasValue(prop_perl_project_preamble))
        {
            WritePropSourceCode(Project.get_ProjectNode(), prop_perl_project_preamble);
        }
    }

    Code code(m_form_node, m_language);
    code.Str("use Wx;").Eol();
    code.Str("package ").NodeName();
    if (code.ends_with("Base"))
    {
        code.erase(code.size() - 4);
    }
    code.Str(";").Eol();

    m_source->writeLine(code);
    m_source->writeLine();
}

auto PerlCodeGenerator::WriteIDConstants() -> void
{
    int id_value = wxID_HIGHEST;
    for (const auto& iter: m_set_enum_ids)
    {
        m_source->writeLine(tt_string() << '$' << iter << " = " << id_value++);
    }
    for (const auto& iter: m_set_const_ids)
    {
        if (tt::contains(iter, " wx"))
        {
            wxString wx_id = '$' + iter;
            wx_id.Replace(" wx", " wx.", true);
            m_source->writeLine(wx_id.ToStdString());
        }
        else
        {
            m_source->writeLine('$' + iter);
        }
    }
}

auto PerlCodeGenerator::WriteSampleFrameApp(Code& code) -> void
{
    if (m_form_node->is_Type(type_frame_form))
    {
        code += txt_perl_frame_app;
        if (m_form_node->HasValue(prop_class_name))
        {
            tt_string class_name = m_form_node->as_string(prop_class_name);
            if (class_name.ends_with("Base"))
            {
                class_name.erase(class_name.size() - 4);
            }
            code.Replace("MainFrame", class_name);
        }
        m_header->writeLine(code);
        code.clear();
    }
}

auto PerlCodeGenerator::GenerateConstructionCode(Code& code) -> void
{
    auto* generator = m_form_node->get_NodeDeclaration()->get_Generator();
    code.clear();
    if (generator->ConstructionCode(code))
    {
        m_source->writeLine(code);
        m_source->writeLine();
        m_source->Indent();
        m_source->Indent();

        int id_value = wxID_HIGHEST;
        for (const auto& iter: m_set_enum_ids)
        {
            m_source->writeLine(tt_string() << '@' << iter << id_value++);
        }

        if (id_value > 1)
        {
            // If at least one id was set, add a blank line
            m_source->writeLine();
        }
    }

    code.clear();
    m_source->ResetIndent(1);
    if (generator->SettingsCode(code))
    {
        if (code.size())
        {
            m_source->writeLine(code);
            m_source->writeLine();
        }
    }

    if (m_form_node->get_PropPtr(prop_window_extra_style))
    {
        code.clear();
        code.GenWindowSettings();
        if (code.size())
        {
            m_source->writeLine(code);
        }
    }

    m_source->SetLastLineBlank();
    for (const auto& child: m_form_node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_wxContextMenuEvent))
        {
            continue;
        }
        GenConstruction(child.get());
    }

    code.clear();
    if (generator->AfterChildrenCode(code))
    {
        if (code.size())
        {
            m_source->writeLine();
            m_source->writeLine(code);
        }
    }

    // TODO: [Randalphwa - 07-13-2023] Need to figure out if wxPerl supports persistence

    // Timer code must be created before the events, otherwise the timer variable won't exist
    // when the event is created.

    code.clear();
    if (TimerGenerator::StartIfChildTimer(m_form_node, code))
    {
        m_source->writeLine(code);
        m_source->writeLine();
    }
}

auto PerlCodeGenerator::GenerateEventHandlers(Code& code, std::thread& thrd_get_events) -> void
{
    // Delay calling join() for as long as possible to increase the chance that the thread will
    // have already completed.
    thrd_get_events.join();
    if (m_events.size())
    {
        m_source->writeLine();
        m_source->writeLine("# Event handlers");
        GenSrcEventBinding(m_form_node, m_events);

        m_source->writeLine("\t# end Event handlers\n", indent::none);
        m_source->SetLastLineBlank();

        m_source->ResetIndent();
        m_source->writeLine();
        m_source->Indent();
        GenUnhandledEvents(m_events);
    }
    else
    {
        m_source->ResetIndent();
    }

    if (m_form_node->is_Gen(gen_wxWizard))
    {
        code.clear();
        code.Eol().Str("# Add the following below the comment block to add a simple");
        code.Eol().Str("# Run() function to launch the wizard").Eol();
        m_source->writeLine(code);
    }
}

auto PerlCodeGenerator::WriteHelperFunctions() -> void
{
    if (m_NeedImageFunction)
    {
        m_source->writeLine(txt_perl_get_bundle, indent::auto_keep_whitespace);
    }
}

auto PerlCodeGenerator::WriteEmbeddedImages(Code& code) -> void
{
    code.clear();
    // Now write any embedded images that aren't declared in the gen_Images List
    for (auto& iter: m_embedded_images)
    {
        // Only write the images that aren't declared in any gen_Images List. Note that
        // this *WILL* result in duplicate images being written to different forms.
        if (iter->get_Form() != m_ImagesForm)
        {
            WriteImageConstruction(code);
            m_source->doWrite("\n");  // force an extra line break
            m_source->SetLastLineBlank();
            break;
        }
    }
}

void PerlCodeGenerator::GenerateClass(GenLang language, PANEL_PAGE panel_type)
{
    m_language = language;
    m_panel_type = panel_type;
    ASSERT(m_language == GEN_LANG_PERL);
    Code code(m_form_node, m_language);

    m_embedded_images.clear();

    SetImagesForm();

    // Start threads early to process in background
    std::set<std::string> img_include_set;
    auto [thrd_get_events, thrd_need_img_func, thrd_collect_img_headers] =
        InitializeThreads(img_include_set);

    // If the code files are being written to disk, then UpdateEmbedNodes() has already been called.
    if (panel_type != NOT_PANEL)
    {
        ProjectImages.UpdateEmbedNodes();
    }

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    m_panel_type = panel_type;

    m_header->Clear();
    m_source->Clear();

    WriteSampleFrameApp(code);
    WriteSourceHeader();

    InitializeUsageStatements();
    WriteUsageStatements();

    m_set_enum_ids.clear();
    m_set_const_ids.clear();
    // Do this early to give threads a chance to run before we need to join them.
    BaseCodeGenerator::CollectIDs(m_form_node, m_set_enum_ids, m_set_const_ids);

    m_source->writeLine();
    m_source->writeLine(
        "use utf8;");  // required since C++, wxPython, and wxRuby use utf8 by default
    m_source->writeLine("use strict;");

    ScriptCommon::JoinThreadSafely(thrd_collect_img_headers);

    if (m_form_node->is_Gen(gen_Images))
    {
        m_source->writeLine("use MIME::Base64;");
        thrd_get_events.join();
        thrd_need_img_func.join();
        GenerateImagesForm();
        return;
    }

    if (m_embedded_images.size())
    {
        CheckMimeBase64Requirement(code);
    }

    WriteIDConstants();
    GenerateConstructionCode(code);
    GenerateEventHandlers(code, thrd_get_events);

    // Make certain indentation is reset after all construction code is written
    m_source->ResetIndent();
    m_source->writeLine("\treturn $self;", indent::none);
    m_source->writeLine("}\n\n", indent::none);

    thrd_need_img_func.join();

    WriteHelperFunctions();

    // Only add this when writing to disk. Otherwise, it needs to be added after
    // the comment block, and only if there is no user code after the comment
    // block. This is to ensure that the user can add event handlers that are
    // part of the package.
    if (panel_type != NOT_PANEL)
    {
        m_source->writeLine("1;", indent::none);
    }

    m_header->ResetIndent();

    WriteEmbeddedImages(code);
}

void PerlCodeGenerator::WriteUsageStatements()
{
    if (m_use_classes.size())
    {
        m_source->writeLine();
        for (const auto& import: m_use_classes)
        {
            m_source->writeLine(import);
        }
        m_source->writeLine();
    }

    if (m_use_classes.size())
    {
        for (const auto& iter: m_use_packages)
        {
            m_source->writeLine(iter);
        }
        m_source->writeLine();
    }

    if (m_use_expands.size())
    {
        for (const auto& iter: m_use_expands)
        {
            m_source->writeLine(iter);
        }
        m_source->writeLine();
    }

    if (m_use_constants.size())
    {
        for (const auto& iter: m_use_constants)
        {
            m_source->writeLine(iter);
        }
        m_source->writeLine();
    }
}

void PerlCodeGenerator::GenerateImagesForm()
{
    if (m_embedded_images.empty() || !m_form_node->get_ChildCount())
    {
        return;
    }

    // TODO: [Randalphwa - 05-29-2025] Implement Perl image generation
}

// This function simply generates unhandled event handlers in a multi-string comment.

auto PerlCodeGenerator::CollectExistingEventHandlers(std::unordered_set<std::string>& code_lines)
    -> bool
{
    return ScriptCommon::CollectExistingEventHandlers(m_form_node, GEN_LANG_PERL, m_panel_type,
                                                      code_lines, "sub ");
}

auto PerlCodeGenerator::GenerateEventHandlerComment(bool found_user_handlers, Code& code) -> void
{
    ScriptCommon::GenerateEventHandlerComment(found_user_handlers, code, GEN_LANG_PERL);
}

auto PerlCodeGenerator::GenerateEventHandlerBody(NodeEvent* event, Code& undefined_handlers) -> void
{
    ScriptCommon::GenerateEventHandlerBody(event, undefined_handlers, GEN_LANG_PERL);
}

auto PerlCodeGenerator::WriteEventHandlers(Code& code, Code& undefined_handlers) -> void
{
    if (undefined_handlers.size())
    {
        m_source->writeLine(code);

        m_source->ResetIndent();
        m_source->writeLine(perl_begin_cmt_block);
        m_source->Indent();
        m_source->writeLine(undefined_handlers);
        m_source->Unindent();
        m_source->writeLine(perl_end_cmt_block);
        m_source->Indent();

        m_header->writeLine(code);
        m_header->writeLine(undefined_handlers);
    }
}

void PerlCodeGenerator::GenUnhandledEvents(EventVector& events)
{
    ASSERT_MSG(events.size(), "GenUnhandledEvents() shouldn't be called if there are no events");
    if (events.empty())
    {
        return;
    }

    // Multiple events can be bound to the same function, so use a set to make sure we only generate
    // each function once.
    std::unordered_set<std::string> code_lines;

    Code code(m_form_node, GEN_LANG_PERL);
    auto sort_event_handlers = [](NodeEvent* event_a, NodeEvent* event_b)
    {
        return (EventHandlerDlg::GetPerlValue(event_a->get_value()) <
                EventHandlerDlg::GetPerlValue(event_b->get_value()));
    };

    // Sort events by function name
    std::ranges::sort(events, sort_event_handlers);

    bool found_user_handlers = CollectExistingEventHandlers(code_lines);

    // Check if all events are already implemented
    bool is_all_events_implemented = true;
    if (found_user_handlers)
    {
        for (auto& event: events)
        {
            auto handler = EventHandlerDlg::GetPerlValue(event->get_value());
            // Ignore lambda's
            if (handler.starts_with("[perl:lambda]"))
            {
                continue;
            }

            tt_string set_code;
            set_code << "sub " << handler << " {";
            if (!code_lines.contains(set_code))
            {
                // At least one event wasn't implemented
                is_all_events_implemented = false;
                break;
            }
        }

        if (is_all_events_implemented)
        {
            // All event handlers are defined, nothing to generate
            return;
        }
    }
    else
    {
        is_all_events_implemented = false;
    }

    GenerateEventHandlerComment(found_user_handlers, code);

    Code undefined_handlers(m_form_node, GEN_LANG_PERL);
    for (auto& event: events)
    {
        auto handler = EventHandlerDlg::GetPerlValue(event->get_value());
        // Ignore lambda's
        if (handler.empty() || handler.starts_with("[perl:lambda]"))
        {
            continue;
        }

        tt_string set_code;
        set_code << "sub " << handler << " {";

        if (code_lines.contains(set_code))
        {
            continue;
        }
        code_lines.emplace(set_code);

        undefined_handlers.Str(set_code).Eol();
        undefined_handlers.Tab().Str("my ($self, $event) = @_;").Eol();
#if defined(_DEBUG)
        const auto& dbg_event_name = event->get_name();
        wxUnusedVar(dbg_event_name);
#endif  // _DEBUG
        GenerateEventHandlerBody(event, undefined_handlers);
        undefined_handlers.Eol();
        undefined_handlers.Str("}").Eol();
    }

    WriteEventHandlers(code, undefined_handlers);
}

auto PerlCodeGenerator::ProcessImageFromImagesForm(Code& code, bool& images_file_imported,
                                                   bool& svg_import_libs, const EmbeddedImage* iter)
    -> void
{
    if (!images_file_imported)
    {
        tt_string import_name = iter->get_Form()->as_string(prop_perl_file).filename();
        import_name.remove_extension();
        code.Eol().Str("use ").Str(import_name) << "'";
        m_source->writeLine(code);
        code.clear();
        images_file_imported = true;
    }

    if (iter->base_image().type == wxBITMAP_TYPE_SVG)
    {
        if (!svg_import_libs)
        {
            // TODO: [Randalphwa - 05-29-2025] Currently, wxPerl does not support SVG
            // files. We could take advantage of wxLunaSVG and convert the SVG files to
            // PNG -- in fact we could even generate multiple resolution PNG files.

            // svg_import_libs = true;
        }
    }
}

auto PerlCodeGenerator::ProcessExternalImage([[maybe_unused]] const EmbeddedImage* iter,
                                             [[maybe_unused]] bool svg_import_libs) -> void
{
    // If the image isn't in the images file, then we need to add the base64 version
    // of the bitmap
    if (!m_base64_requirement_written)
    {
        m_source->writeLine("use MIME::Base64;");
        m_base64_requirement_written = true;
    }

    // At this point we know that some method is required, but until we have
    // processed all the images, we won't know if the images file is required. The
    // images file provides it's own function for loading images, so we can use that
    // if it's available.
    m_NeedImageFunction = true;
}

void PerlCodeGenerator::CheckMimeBase64Requirement(Code& code)
{
    ASSERT_MSG(m_embedded_images.size(),
               "CheckMimeBase64Requirement() should only be called if there are embedded images");
    if (m_embedded_images.empty())
    {
        return;
    }

    bool images_file_imported = false;
    bool svg_import_libs = false;

    for (auto& iter: m_embedded_images)
    {
        if (iter->get_Form() == m_ImagesForm)
        {
            ProcessImageFromImagesForm(code, images_file_imported, svg_import_libs, iter);
        }

        if (iter->get_Form() != m_ImagesForm)
        {
            ProcessExternalImage(iter, svg_import_libs);
        }
    }

    if (m_NeedImageFunction)
    {
        if (images_file_imported)
        {
            // The images file supplies the function we need
            m_NeedImageFunction = false;
        }
        else
        {
            // REVIEW: [Randalphwa - 05-29-2025] This is just here as a placeholder in case we
            // need to bring in another package for loading images.
            if (!m_stringio_requirement_written)
            {
                // No further check for this is needed
                // stringio_requirement_written = true;
                // m_source->writeLine("require 'stringio'");
            }
        }
    }
}

void PerlCodeGenerator::InitializeUsageStatements()
{
    m_use_expands.emplace("use Wx qw[:id];");

    ParseNodesForUsage(m_form_node);

    if (m_art_ids.size())
    {
        constexpr auto art_provider_overhead = 15;  // Space needed for "use Wx::ArtProvider qw();"
        tt_string art_ids;
        for (const auto& iter: m_art_ids)
        {
            if (art_ids.empty())
            {
                art_ids = "use Wx::ArtProvider qw(";
            }
            else
            {
                art_ids += ' ';
            }
            art_ids += iter;
            if (art_ids.size() > (Project.as_size_t(prop_perl_line_length) - art_provider_overhead))
            {
                art_ids += ");";
                m_use_packages.emplace(art_ids);
                art_ids.clear();
            }
        }
        if (art_ids.size())
        {
            art_ids += ");";
            m_use_packages.emplace(art_ids);
        }
    }
}

auto PerlCodeGenerator::ProcessNodeProperties(Node* node) -> void
{
    if (node->HasValue(prop_window_style) || node->HasValue(prop_window_extra_style) ||
        (node->HasValue(prop_variant) && node->as_string(prop_variant) != "normal"))
    {
        m_use_expands.emplace("use Wx qw[:window];");
    }

    if (node->HasValue(prop_bitmap))
    {
        if (node->as_string(prop_bitmap).contains("wxART_"))
        {
            m_use_packages.emplace("use Wx::ArtProvider qw[:artid :clientid];");
        }
        else if (node->as_string(prop_bitmap).starts_with("XPM"))
        {
            m_use_expands.emplace("use Wx qw[:image];");
        }
    }

    if (node->HasValue(prop_font))
    {
        m_use_expands.emplace("use Wx qw[:font];");
        FontProperty fontprop(node->get_PropPtr(prop_font));
        if (fontprop.isDefGuiFont())
        {
            // If the font is a default GUI font, then we need to include the wxDefaultGuiFont
            // constant.
            m_use_expands.emplace("use Wx qw[:systemsettings];");
        }
    }

    if (node->HasValue(prop_foreground_colour) || node->HasValue(prop_background_colour))
    {
        if (node->as_string(prop_foreground_colour).contains("wxSYS") ||
            node->as_string(prop_background_colour).contains("wxSYS"))
        {
            m_use_expands.emplace("use Wx qw[:systemsettings];");
        }
    }
}

auto PerlCodeGenerator::ProcessNodeImports(Node* node) -> void
{
    auto* gen = node->get_Generator();
    if (!gen)
    {
        return;
    }

    std::set<std::string> imports;
    gen->GetImports(node, imports, GEN_LANG_PERL);
    for (const auto& iter: imports)
    {
        if (iter.starts_with("use Wx qw[:"))
        {
            m_use_expands.emplace(iter);
        }
        else if (iter.starts_with("use Wx::"))
        {
            m_use_packages.emplace(iter);
        }
        else if (iter.starts_with("use Wx "))
        {
            m_use_constants.emplace(iter);
        }
        else
        {
            m_use_classes.emplace(iter);
        }
    }

    for (const auto& iter: map_perl_constants)
    {
        if (node->HasProp(iter.first))
        {
            tt_string constants("use Wx qw(");
            constants += iter.second;
            constants += ");";
            m_use_constants.emplace(constants);
        }
    }
}

void PerlCodeGenerator::ParseNodesForUsage(Node* node)
{
    if (node->is_Sizer())
    {
        m_use_expands.emplace("use Wx qw[:sizer];");

        // Now recurse through any children and their children
        for (auto& child: node->get_ChildNodePtrs())
        {
            ParseNodesForUsage(child.get());
        }
        return;
    }

    ProcessNodeProperties(node);
    ProcessNodeImports(node);

    // Now recurse through any children and their children
    for (auto& child: node->get_ChildNodePtrs())
    {
        ParseNodesForUsage(child.get());
    }
}

auto MakePerlPath(Node* node) -> tt_string
{
    return ScriptCommon::MakeScriptPath(node, GEN_LANG_PERL);
}

[[nodiscard]] auto HasPerlMapConstant(std::string_view value) -> bool
{
    return std::ranges::any_of(map_perl_constants,
                               [&](const auto& iter)
                               {
                                   return tt::contains(iter.second, value);
                               });
}

auto PerlBitmapList(Code& code, GenEnum::PropName prop) -> bool
{
    const auto& description = code.node()->as_string(prop);
    ASSERT_MSG(description.size(), "PerlBitmapList called with empty description");
    tt_view_vector parts(description, BMP_PROP_SEPARATOR, tt::TRIM::both);

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

    bool is_xpm = (parts[IndexType].is_sameas("XPM"));
    auto path = MakePerlPath(code.node());

    code += "bitmaps = [ ";
    bool needs_comma = false;
    for (const auto& iter: bundle->lst_filenames)
    {
        if (needs_comma)
        {
            code.UpdateBreakAt();
            code.Comma(false).Eol().Tab(3);
        }

        bool is_embed_success = false;
        if (parts[IndexType].starts_with("Embed"))
        {
            if (auto* embed = ProjectImages.GetEmbeddedImage(iter); embed)
            {
                code.AddPerlImageName(embed);
                code += ".Bitmap";
                needs_comma = true;
                is_embed_success = true;
            }
        }

        if (!is_embed_success)
        {
            tt_string name(iter);
            name.make_absolute();
            name.make_relative(path);
            name.backslashestoforward();

            code.Str("Wx::Bitmap->new(").QuotedString(name);
            if (is_xpm)
            {
                code.Comma().Str("wxBITMAP_TYPE_XPM");
            }
            code += ")";
            needs_comma = true;
        }
    }
    code += " ]\n";
    code.UpdateBreakAt();

    return true;
}
