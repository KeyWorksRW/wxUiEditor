// Purpose:   Generates base class for wxDocument/wView applications
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_doc_view_app.h"

#include "code.h"  // Code -- Helper class for generating code

inline constexpr const auto txt_DocViewAppHeader =
    R"===("// Base class for wxDocument/wxView applications.
// App class should inherit from this in addition to wxApp.

// In your app's OnRun() function, call this class's Create() function to
// create the main frame, and then call Show() to display it. Do this before
// returning wxApp::OnRun();

// In your app's OnExit() function, call this class's PrepForExit() function to
// save the file history and delete the document manager. Do this before
// returning wxApp::OnExit();

#include <vector>

class wxFrame;
class wxDocManager;
class wxMenuBar;
class wxDocTemplate;

// Yout application's App class should inherit from this in addition to wxApp, e.g.
//     class App : public wxApp, public DocViewApp
class %class%
{
public:
    wxFrame* Create(wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE, const wxString& name = wxFrameNameStr);

    // Call this from the Application's OnExit() function. It will save the
    // file history and delete the document manager.
    void PrepForExit();

    auto GetFrame() const { return m_frame; }
    wxDocManager* GetDocumentManager() const { return m_docManager; }
    wxMenuBar* GetMenuBar() const { return m_menuBar; }
    auto GetDocTemplates() const { return m_docTemplates; }

    wxFrame* CreateChildFrame(wxView* view);

    bool Show(bool show = true) { return m_frame->Show(show); }

protected:
    wxFrame* m_frame { nullptr };
    wxDocManager* m_docManager { nullptr };
    wxMenuBar* m_menuBar { nullptr };

    std::vector<wxDocTemplate*> m_docTemplates;
};
)===";

inline constexpr const auto txt_DocViewAppCppSrc =
    R"===("

wxFrame* %class%::Create(wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style,
                                const wxString& name)
{
    m_docManager = new wxDocManager;

    %doc_templates%

    m_frame = new wxDocParentFrameAny<wxAuiMDIParentFrame>(m_docManager, nullptr, id, title, pos, size, style, name);

    m_menuBar = new wxMenuBar;
    %default_menu%
    m_frame->SetMenuBar(m_menuBar);

    return m_frame;
}

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
    child_frame->SetIcon(wxICON(notepad));

    return child_frame;
}

void %class%::PrepForExit()
{
    m_docManager->FileHistorySave(*wxConfig::Get());
    delete m_docManager;
}
)===";

bool DocViewAppGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        tt_string_vector lines;
        lines.ReadString(txt_DocViewAppCppSrc);
        tt_string class_name = code.node()->value(prop_class_name);
        for (auto& line: lines)
        {
            line.Replace("%class%", class_name, true);
            code.Str(line).Eol();
        }
    }

    return true;
}

bool DocViewAppGenerator::GetIncludes(Node* /* node */, std::set<std::string>& set_src, std::set<std::string>& /* set_hdr */)
{
    set_src.insert("#include <wx/aui/tabmdi.h");
    set_src.insert("#include <wx/config.h");
    set_src.insert("#include <wx/docmdi.h");
    set_src.insert("#include <wx/docview.h");
    set_src.insert("#include <wx/menu.h");

    return true;
}
