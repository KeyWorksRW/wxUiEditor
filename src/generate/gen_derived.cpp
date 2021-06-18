/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate the derived class source and header file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <thread>

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "gen_base.h"

#include "mainapp.h"       // App -- App class
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "write_code.h"    // Write code to Scintilla or file

// clang-format off

inline constexpr const auto txt_DerivedCmtBlock =
R"===(////////////////////////////////////////////////////////////////////////////
// Original Code generated by wxUiEditor
//
// The code is generated only once. You can now modify the code as needed.
////////////////////////////////////////////////////////////////////////////

)===";

// clang-format on

int BaseCodeGenerator::GenerateDerivedClass(Node* project, Node* form, PANEL_TYPE panel_type)
{
    ttlib::cstr source_ext(".cpp");
    ttlib::cstr header_ext(".h");

    if (auto& extProp = project->prop_as_string(prop_source_ext); extProp.size())
    {
        source_ext = extProp;
    }

    if (auto& extProp = project->prop_as_string(prop_header_ext); extProp.size())
    {
        header_ext = extProp;
    }

    ttlib::cstr derived_file;
    if (auto& file_prop = form->prop_as_string(prop_derived_file); file_prop.size())
    {
        derived_file = file_prop;
        if ((derived_file.empty() || derived_file.is_sameas("filename")) && panel_type == NOT_PANEL)
            return result::ignored;
        derived_file.make_relative(wxGetApp().getProjectPath());
        derived_file.backslashestoforward();
    }
    else
    {
        // If there is no derived_file property, then there's nothing we can write to
        return result::ignored;
    }

    // If the user cleared the property, change the name to a default value for displaying in the derived panel.
    if (derived_file.empty())
        derived_file = "derived_file";

    EventVector events;
    std::thread thrd_get_events(&BaseCodeGenerator::CollectEventHandlers, this, form, std::ref(events));

    derived_file.replace_extension(source_ext);
    if (panel_type == NOT_PANEL && derived_file.file_exists())
        return result::exists;  // We never allow writing over an existing derived class file

    derived_file.remove_extension();

    ttlib::cstr baseFile;
    if (auto& file = form->prop_as_string(prop_base_file); file.size())
    {
        baseFile = file;
        baseFile.replace_extension(header_ext);
        ttlib::cstr root(derived_file);
        root.remove_filename();
        if (root.size())
            baseFile.make_relative(root);
        baseFile.backslashestoforward();
        baseFile.remove_extension();
    }

    ttlib::cstr namespace_using_name;

    // Make a copy of the string so that we can tweak it
    if (auto namespace_prop = project->prop_as_string(prop_name_space); namespace_prop.size())
    {
        // ttlib::multistr works with a single char, not a string.
        namespace_prop.Replace("::", ":");
        // we also accept using semi-colons to separate the namespaces
        namespace_prop.Replace(";", ":");

        ttlib::multistr names(namespace_prop, ':');
        for (auto& iter: names)
        {
            if (namespace_using_name.size())
            {
                namespace_using_name += "::";
            }
            namespace_using_name += iter;
        }
    }

    auto derived_name = form->prop_as_string(prop_derived_class_name);

    m_header->Clear();
    m_source->Clear();

    m_source->writeLine(txt_DerivedCmtBlock);

    if (panel_type != CPP_PANEL)
    {
        m_header->writeLine(txt_DerivedCmtBlock);

        m_header->writeLine("\n#pragma once");
        m_header->writeLine();

        baseFile.replace_extension(header_ext);
        m_header->writeLine(ttlib::cstr().Format("#include %ks", baseFile.c_str()));
        baseFile.remove_extension();
        m_header->writeLine();

        ttlib::cstr line;
        line << "class " << derived_name << " : public ";
        if (namespace_using_name.size())
        {
            line << namespace_using_name << "::";
        }

        line << form->get_node_name();

        m_header->writeLine(line);
        m_header->writeLine("{");

        m_header->writeLine("public:");
        m_header->Indent();

        m_header->writeLine(ttlib::cstr() << derived_name << "(wxWindow* parent = nullptr);");

        m_header->Unindent();
    }

    if (panel_type != HDR_PANEL)
    {

        if (auto prop = project->prop_as_string(prop_local_pch_file); prop.size())
        {
            ttlib::cstr pch("#include ");
            pch << "\"" << prop << "\"";

            m_source->writeLine();
            m_source->writeLine(pch);
            m_source->writeLine();
        }

        if (project->HasValue(prop_src_preamble))
        {
            ttlib::cstr code(project->prop_as_string(prop_src_preamble));

            // The multi-line editor may have been used in which case there are escaped newlines and tabs -- we convert
            // those to the actual characters before generating the code. It's common with that editor to have a trailing
            // EOL -- so we remove that if needed.
            code.Replace("\\n", "\n", true);
            code.Replace("\\t", "\t", true);
            if (code.back() == '\n')
                code.erase(code.size() - 1, 1);
            m_source->writeLine(code);
            m_source->writeLine();
        }

        {
            ttlib::cstr inc;

            // Add a comment to the header the specifies the generated header and source filenames
            baseFile.replace_extension(header_ext);
            derived_file.replace_extension(header_ext);
            inc.Format("#include %kv", derived_file.subview());

            ttlib::cstr comment(ttlib::cstr(header_ext) << "\"  // auto-generated: ");
            comment << baseFile << " and ";
            baseFile.replace_extension(source_ext);
            comment << baseFile;
            inc.Replace(ttlib::cstr(header_ext) << '"', comment);

            if (header_ext != ".h")
                inc.Replace(".h", header_ext);
            m_source->writeLine();
            m_source->writeLine(inc);
            m_source->writeLine();
        }

        if (namespace_using_name.size())
        {
            m_source->writeLine(ttlib::cstr() << "using namespace " << namespace_using_name << ';');
            m_source->writeLine();
        }

        {
            ttlib::cstr code;
            code << derived_name << "::" << derived_name << "(wxWindow* parent) : ";
            code << form->get_node_name() << "(parent) {}";
            m_source->writeLine(code);
        }
    }

    thrd_get_events.join();
    if (events.size() > 0)
    {
        m_header->Unindent();
        m_header->writeLine();
        m_header->writeLine("protected:");
        m_header->Indent();
        m_header->SetLastLineBlank();
        m_header->writeLine(ttlib::cstr() << "// Handlers for " << form->get_node_name() << " events");

        std::set<std::string> generatedHandlers;
        for (size_t i = 0; i < events.size(); i++)
        {
            auto event = events[i];

            // Ignore lambda's and functions in another class
            if (event->get_value().contains("[") || event->get_value().contains("::"))
                continue;

            if (generatedHandlers.find(event->get_value()) == generatedHandlers.end())
            {
                ttlib::cstr prototype;
                if (event->GetEventInfo()->get_name() == "OnInitDialog" || form->prop_as_bool(prop_persist))
                {
                    // OnInitDialog needs to call event.Skip() in order to initialize validators and update the UI
                    prototype.Format("%s(%s& event)", event->get_value().c_str(),
                                     event->GetEventInfo()->get_event_class().c_str());
                }
                else
                {
                    prototype.Format("%s(%s& WXUNUSED(event))", event->get_value().c_str(),
                                     event->GetEventInfo()->get_event_class().c_str());
                }
                m_header->writeLine(ttlib::cstr().Format("void %s override;", prototype.c_str()));

                if (panel_type != HDR_PANEL)
                {
                    m_source->writeLine();
                    m_source->writeLine(ttlib::cstr() << "void " << derived_name << "::" << prototype);
                    m_source->writeLine("{");
                    m_source->Indent();
                    auto name = event->GetEventInfo()->get_name();
                    if (name == "OnInitDialog")
                    {
                        m_source->writeLine("event.Skip();  // transfer all validator data to their windows and update UI");
                    }
                    else if (name == "OnOKButtonClick")
                    {
                        m_source->writeLine("if (!Validate() || !TransferDataFromWindow())");
                        m_source->Indent();
                        m_source->writeLine("return;");
                        m_source->Unindent();
                        m_source->writeLine();
                        m_source->writeLine("if (IsModal())");

                        m_source->Indent();
                        m_source->writeLine("EndModal(wxID_OK);");
                        m_source->Unindent();

                        m_source->writeLine("else");
                        m_source->writeLine("{");

                        m_source->Indent();
                        m_source->writeLine("    SetReturnCode(wxID_OK);");
                        m_source->writeLine("    Show(false);");
                        m_source->Unindent();

                        m_source->writeLine("}");

                        if (form->prop_as_bool(prop_persist))
                        {
                            m_source->writeLine();
                            m_source->writeLine("event.Skip();  // This must be called for wxPersistenceManager to work");
                        }
                    }
                    else
                    {
                        m_source->writeLine(ttlib::cstr().Format("    // TODO: Implement %s", event->get_value().c_str()),
                                            indent::auto_no_whitespace);
                    }
                    m_source->Unindent();

                    m_source->writeLine("}");
                }

                generatedHandlers.insert(event->get_value());
            }
        }
    }

    m_header->Unindent();
    m_header->writeLine("};");

    return result::created;
}
