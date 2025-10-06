// Purpose:   Generates base class for wxDocument/wView applications
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "gen_doc_view_app.h"

#include "code.h"  // Code -- Helper class for generating code

inline constexpr const auto txt_DocViewAppCppSrc =
    R"===(%class%::%class%() : m_frame(nullptr), m_docManager(nullptr), m_menuBar(nullptr)
{
    m_docManager = new wxDocManager;
}

void %class%::AddDocTemplate(const wxString& descr, const wxString& filter, const wxString& dir,
                                const wxString& ext, const wxString& docTypeName,
                                const wxString& viewTypeName, wxClassInfo* docClassInfo,
                                wxClassInfo* viewClassInfo, long flags)
{
    new wxDocTemplate(m_docManager, descr, filter, dir, ext, docTypeName, viewTypeName,
                      docClassInfo, viewClassInfo, flags);
}

int %class%::OnRun()
{
    CreateFrame(wxID_ANY, GetAppDisplayName(), wxDefaultPosition, wxSize(500, 400));
    ShowFrame();

    return wxApp::OnRun();
}

int %class%::OnExit()
{
    m_docManager->FileHistorySave(*wxConfig::Get());
    delete m_docManager;
    return wxApp::OnExit();
}

wxFrame* %class%::CreateFrame(wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style,
                                const wxString& name)
{
    %doc_templates%

    m_frame = new wxDocParentFrameAny<wxAuiMDIParentFrame>(m_docManager, nullptr, id, title, pos, size, style, name);

    auto menuFile = new wxMenu;
    menuFile->Append(wxID_NEW);
    menuFile->Append(wxID_OPEN);

    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    m_docManager->FileHistoryUseMenu(menuFile);
    m_docManager->FileHistoryLoad(*wxConfig::Get());

    auto help = new wxMenu;
    help->Append(wxID_ABOUT);

    m_menuBar = new wxMenuBar;
    m_menuBar->Append(menuFile, wxGetStockLabel(wxID_FILE));
    m_menuBar->Append(help, wxGetStockLabel(wxID_HELP));
    m_frame->SetMenuBar(m_menuBar);

    return m_frame;

)===";

inline constexpr const auto txt_DocViewAppAfterCtor =
    R"===(
wxFrame* %class%::CreateChildFrame(wxView* view)
{
    auto doc = view->GetDocument();
    auto child_frame = new wxDocChildFrameAny<wxAuiMDIChildFrame, wxAuiMDIParentFrame>(
        doc, view, static_cast<wxDocParentFrameAny<wxAuiMDIParentFrame>*>(m_frame), wxID_ANY, "Child Frame",
        wxDefaultPosition, wxSize(300, 300));

    auto menuFile = new wxMenu;

    menuFile->Append(wxID_NEW);
    menuFile->Append(wxID_OPEN);
    menuFile->Append(wxID_CLOSE);
    menuFile->Append(wxID_SAVE);
    menuFile->Append(wxID_SAVEAS);
    menuFile->Append(wxID_REVERT, "Re&vert...");

    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    doc->GetDocumentManager()->FileHistoryAddFilesToMenu(menuFile);

    auto menuEdit = new wxMenu;
    menuEdit->Append(wxID_COPY);
    menuEdit->Append(wxID_PASTE);
    menuEdit->Append(wxID_SELECTALL);

    auto menubar = new wxMenuBar;
    %document_menu%
    m_frame->SetMenuBar(child_frame);
    child_frame->SetMenuBar(menubar);

    return child_frame;
}
)===";

bool DocViewAppGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        std::vector<std::string> doc_templates;
        Code code_templates = code;

        for (auto& child: code.node()->get_ChildNodePtrs())
        {
            if (!child->is_Type(type_wx_document))
            {
                continue;
            }
            code_templates.Indent();

            code_templates.Eol()
                .Str("new wxDocTemplate(m_docManager")
                .Comma()
                .QuotedString(child->as_string(prop_template_description))
                .Comma()
                .QuotedString(child->as_string(prop_template_filter))
                .Comma()
                .QuotedString(child->as_string(prop_template_directory))
                .Comma()
                .QuotedString(child->as_string(prop_template_extension))
                .Comma();
            code_templates.Indent();
            code_templates.Eol()
                .QuotedString(child->as_string(prop_template_doc_name))
                .Comma()
                .QuotedString(child->as_string(prop_template_view_name))
                .Comma();
            code_templates.Eol()
                .Str("CLASSINFO(")
                .Str(child->as_string(prop_derived_class_name))
                .Str(")")
                .Comma();
            for (auto& doc_child: child->get_ChildNodePtrs())
            {
                if (!doc_child->is_Type(type_wx_view))
                {
                    continue;
                }
                code_templates.Str("CLASSINFO(")
                    .Str(doc_child->as_string(prop_derived_class_name))
                    .Str(")");
                break;
            }
            code_templates.EndFunction();
        }
        code_templates.Unindent();

        tt_string_vector lines;
        lines.ReadString(txt_DocViewAppCppSrc);
        tt_string class_name = code.node()->as_string(prop_class_name);

        bool is_mdi = code.node()->as_string(prop_kind) == "MDI";
        for (auto& line: lines)
        {
            line.Replace("%doc_templates%", code_templates, false);
            line.Replace("%class%", class_name, true);
            if (is_mdi)
            {
                line.Replace("wxAuiMDIChildFrame", "wxDocMDIChildFrame", true);
                line.Replace("wxAuiMDIParentFrame", "wxMDIParentFrame", true);
            }
            code.Str(line).Eol();
        }
    }

    return true;
}

auto DocViewAppGenerator::AfterConstructionCode(Code& code) -> bool
{
    if (code.is_cpp())
    {
        tt_string_vector lines;
        lines.ReadString(txt_DocViewAppAfterCtor);
        tt_string class_name = code.node()->as_string(prop_class_name);
        bool is_mdi = code.node()->as_string(prop_kind) == "MDI";
        for (auto& line: lines)
        {
            line.Replace("%class%", class_name, true);
            if (is_mdi)
            {
                line.Replace("wxAuiMDIChildFrame", "wxDocMDIChildFrame", true);
                line.Replace("wxAuiMDIParentFrame", "wxMDIParentFrame", true);
            }
            code.Str(line).Eol();
        }
    }

    return true;
}

auto DocViewAppGenerator::BaseClassNameCode(Code& code) -> bool
{
    code.Str("wxApp");
    return true;
}

inline constexpr const auto txt_DocViewPreAppHeader =
    R"===(// Base class for wxDocument/wxView applications.
// App class should inherit from this in addition to wxApp.

// In your app's OnRun() function, call this class's CreateFrame() function to
// create the main frame, and then call Show() to display it. Do this before
// returning wxApp::OnRun();

// In your app's OnExit() function, call this class's PrepForExit() function to
// save the file history and delete the document manager. Do this before
// returning wxApp::OnExit();

class wxDocManager;
class wxDocTemplate;
class wxFrame;
class wxMenuBar;

)===";

auto DocViewAppGenerator::PreClassHeaderCode(Code& code) -> bool
{
    if (code.is_cpp())
    {
        code += txt_DocViewPreAppHeader;
        return true;
    }

    return false;
}

inline constexpr const auto txt_DocViewAppHeader =
    R"===(virtual void AddDocTemplate(const wxString& descr, const wxString& filter,
                        const wxString& dir, const wxString& ext, const wxString& docTypeName,
                        const wxString& viewTypeName, wxClassInfo* docClassInfo,
                        wxClassInfo* viewClassInfo, long flags);

// This will call CreateFrame(), ShowFrame() and then call wxApp::OnRun(). You do not need to
// override OnRun() in your derived class unless you need to do something additional.
virtual int OnRun() override;

// This will create a DocManager, add templates to it, hook up a file history to it and
// create a menu bar and a main frame.
virtual wxFrame* CreateFrame(wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE, const wxString& name = wxFrameNameStr);

// If you override OnExit() in your derived class, be sure to call this base class's
// OnExit() function to save the file history and delete the document manager. The base
// class's OnExit() returns the value from wxApp::OnExit(), so unless you need additional
// OnExit() processing, you don't need to create your own OnExit() function.
virtual int OnExit() override;

wxFrame* GetFrame() const { return m_frame; }
wxDocManager* GetDocumentManager() const { return m_docManager; }
wxMenuBar* GetMenuBar() const { return m_menuBar; }
auto GetDocTemplates() const { return m_docTemplates; }

virtual wxFrame* CreateChildFrame(wxView* view);

bool Show(bool show = true) { return m_frame->Show(show); }

)===";

auto DocViewAppGenerator::HeaderCode(Code& code) -> bool
{
    tt_string_vector lines;
    lines.ReadString(txt_DocViewAppHeader);
    tt_string class_name = code.node()->as_string(prop_class_name);
    for (auto& line: lines)
    {
        line.Replace("%class%", class_name, true);
        code.Str(line).Eol();
    }

    return true;
}

bool DocViewAppGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                      std::set<std::string>& set_hdr, GenLang language)
{
    if (language == GEN_LANG_CPLUSPLUS)
    {
        if (node->as_string(prop_kind) == "AUI")
        {
            set_src.insert("#include <wx/aui/tabmdi.h");
        }
        set_src.insert("#include <wx/config.h");
        set_src.insert("#include <wx/docmdi.h");
        set_src.insert("#include <wx/menu.h");

        set_hdr.insert("#include <wx/docview.h");
        set_hdr.insert("#include <vector>");

        return true;
    }

    return false;
}
