/////////////////////////////////////////////////////////////////////////////
// Purpose:   DataHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "data_handler.h"

#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes
#include <wx/xml/xml.h>   // wxXmlDocument - XML parser & data holder class
#include <wx/zstream.h>   // zlib stream classes

#include <format>
#include <fstream>

#include <pugixml.hpp>

#include "code.h"             // Code -- Helper class for generating code
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Miscellaneous utility functions
#include "write_code.h"       // Write code to Scintilla or file

DataHandler& ProjectData = DataHandler::getInstance();

auto DataHandler::Initialize() -> void
{
    auto* node_data_list = data_list::FindDataList();
    if (!node_data_list)
    {
        return;
    }

    if (m_embedded_data.size())
    {
        // Ideally, this shouldn't be necessary, but just in case this removes any entry that
        // doesn't have a matching node.

        std::set<wxue::string> var_names;
        for (const auto& node: node_data_list->get_ChildNodePtrs())
        {
            var_names.insert(node->as_string(prop_var_name));
        }

        for (auto iter = m_embedded_data.begin(); iter != m_embedded_data.end();)
        {
            if (var_names.contains(iter->first))
            {
                ++iter;
                continue;
            }
            m_embedded_data.erase(iter);
            if (m_embedded_data.empty())
            {
                break;
            }
            iter = m_embedded_data.begin();
        }
    }

    auto rlambda = [&](Node* parent, auto&& rlambda) -> void
    {
        for (const auto& node: parent->get_ChildNodePtrs())
        {
            if (node->is_Gen(gen_data_folder))
            {
                rlambda(node.get(), rlambda);
                continue;
            }
            if (m_embedded_data.contains(node->as_string(prop_var_name)))
            {
                // If the filename is empty, there's nothing to load.
                if (node->as_string(prop_data_file).empty())
                {
                    continue;
                }

                auto& embed = m_embedded_data[node->as_string(prop_var_name)];

                if (embed.filename == node->as_string(prop_data_file) && embed.type != wxue::npos)
                {
                    // If it's an XML file, then don't continue if xml_condensed has changed
                    if (!node->is_Gen(gen_data_xml) ||
                        node->as_bool(prop_xml_condensed_format) == embed.xml_condensed)
                    {
                        continue;
                    }
                }

                // If we get here, the variable name and filename was specified, but either the
                // filename changed or it could not be found. Calling LoadAndCompress() will
                // replace the EmbeddedData structure.
            }
            LoadAndCompress(node.get());
        }
    };

    rlambda(node_data_list, rlambda);
}

auto DataHandler::LoadAndCompress(Node* node) -> bool
{
    ASSERT(node->is_Gen(gen_data_string) || node->is_Gen(gen_data_xml));
    m_embedded_data[node->as_string(prop_var_name)] = {};
    auto& embed = m_embedded_data[node->as_string(prop_var_name)];
    embed.array_size = 0;
    embed.array_data.clear();
    embed.type = wxue::npos;

    auto filename = node->as_string(prop_data_file);
    if (!filename.size())
    {
        embed.filename = "// No filename specified";
        return false;
    }

    auto [path, has_base_file] = Project.GetOutputPath(node->get_Parent(), GEN_LANG_CPLUSPLUS);
    if (has_base_file)
    {
        // true if the the base filename was returned, in which case we need to convert the
        // relative path to the output path to a relative path to the project path
        path.remove_filename();
        path.append(filename);
        filename = path;
        filename.make_absolute();
        filename.make_relative(Project.get_ProjectPath());
    }

    if (!filename.file_exists())
    {
        auto project_path = Project.get_ProjectPath();
        project_path.remove_filename();
        project_path.append(filename);
        if (!filename.file_exists())
        {
            return false;
        }
        embed.filename = project_path;
    }
    embed.filename = filename;
    embed.filename.backslashestoforward();

    if (node->is_Gen(gen_data_xml) && node->as_bool(prop_xml_condensed_format))
    {
        pugi::xml_document doc;
        if (auto result =
                doc.load_file_string(embed.filename, pugi::parse_trim_pcdata | pugi::parse_default);
            !result)
        {
            if (auto* frame = wxGetMainFrame(); frame != nullptr)
            {
                wxMessageDialog(frame, result.detailed_msg, "Parsing Error", wxOK | wxICON_ERROR)
                    .ShowModal();
            }
            return false;
        }
        std::ostringstream xml_stream;
        doc.save(xml_stream, "", pugi::format_raw | pugi::format_no_declaration);
        std::string str = xml_stream.str();

        // Include the trailing zero -- we need to read this back as a string, not a data array
        wxMemoryInputStream stream(str.c_str(), str.size() + 1);

        wxMemoryOutputStream memory_stream;
        wxZlibOutputStream save_strem(memory_stream, wxZ_BEST_COMPRESSION);
        if (!CopyStreamData(&stream, &save_strem, stream.GetLength()))
        {
            // TODO: [KeyWorks - 03-16-2022] This would be really bad, though it should be
            // impossible
            return false;
        }
        save_strem.Close();
        size_t org_size = (str.size() & 0xFFFFFFFF);
        size_t compressed_size = memory_stream.TellO();
        auto* read_stream = memory_stream.GetOutputStreamBuffer();

        embed.type = 1;
        embed.xml_condensed = true;
        embed.array_size = (compressed_size | (org_size << 32));
        embed.array_data.resize(compressed_size);
        memcpy(embed.array_data.data(), read_stream->GetBufferStart(), compressed_size);
        embed.file_time = embed.filename.last_write_time();
        return true;
    }

    if (!node->as_bool(prop_no_compression))
    {
        std::ifstream file(embed.filename, std::ios::binary | std::ios::ate);
        if (file.is_open())
        {
            std::streampos fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            std::vector<char> buffer(fileSize);
            file.read(buffer.data(), fileSize);
            file.close();

            // Include the trailing zero -- we need to read this back as a string, not a data array
            wxMemoryInputStream stream(buffer.data(), fileSize);

            wxMemoryOutputStream memory_stream;
            wxZlibOutputStream save_strem(memory_stream, wxZ_BEST_COMPRESSION);
            if (!CopyStreamData(&stream, &save_strem, stream.GetLength()))
            {
                // TODO: [KeyWorks - 03-16-2022] This would be really bad, though it should be
                // impossible
                return false;
            }
            save_strem.Close();
            size_t org_size = (buffer.size() & 0xFFFFFFFF);
            size_t compressed_size = memory_stream.TellO();
            auto* read_stream = memory_stream.GetOutputStreamBuffer();

            embed.type = node->is_Gen(gen_data_xml) ? 1 : 0;
            embed.xml_condensed = false;
            embed.array_size = (compressed_size | (org_size << 32));
            embed.array_data.resize(compressed_size);
            memcpy(embed.array_data.data(), read_stream->GetBufferStart(), compressed_size);
            embed.file_time = embed.filename.last_write_time();
            return true;
        }
    }
    else
    {
        wxFFileInputStream stream(embed.filename);
        if (!stream.IsOk())
        {
            return false;
        }
        embed.type = 0;
        embed.array_size = static_cast<size_t>(stream.GetLength());
        embed.array_data.resize(embed.array_size);
        stream.Read(embed.array_data.data(), embed.array_size);
        embed.file_time = embed.filename.last_write_time();
        return true;
    }

    return false;
}

auto DataHandler::WriteDataConstruction(Code& code, WriteCode* source) -> void
{
    // Make certain all files have been loaded
    Initialize();

    size_t processed_count = 0;
    for (const auto& node: code.node()->get_ChildNodePtrs())
    {
        if (m_embedded_data.contains(node->as_string(prop_var_name)))
        {
            auto& embed = m_embedded_data[node->as_string(prop_var_name)];

            // Since we've already called Initialize(), all valid files should have been
            // loaded, so if type is wxue::npos then there's no file to load, or it can't be
            // loaded correctly.
            if (embed.type == wxue::npos)
            {
                continue;
            }

            auto file_time = embed.filename.last_write_time();
            if (file_time == embed.file_time)
            {
                continue;
            }
            ++processed_count;
            if (auto* frame = wxGetMainFrame(); frame)
            {
                frame->setStatusText(std::format("Processing data entry {}: {}", processed_count,
                                                 node->as_string(prop_var_name).c_str()));
            }
            LoadAndCompress(node.get());
        }
    }
    if (processed_count > 0)
    {
        if (auto* frame = wxGetMainFrame(); frame)
        {
            frame->setStatusText(
                std::format("Completed processing {} data entries", processed_count));
        }
    }

    ASSERT_MSG(code.is_cpp(), "This function is only used for C++ code generation");
    code.clear();
    // -12 to account for 8 indent + max 3 chars for number + comma
    size_t cpp_line_length = Project.as_size_t(prop_cpp_line_length) - 12;

    processed_count = 0;
    for (auto& iter_array: m_embedded_data)
    {
        auto& var_name = iter_array.first;
        auto& embed = iter_array.second;
        if (embed.type == wxue::npos)
        {
            continue;
        }
        ++processed_count;
        if (auto* frame = wxGetMainFrame(); frame)
        {
            frame->setStatusText(
                std::format("Writing data array {}: {}", processed_count, var_name.c_str()));
        }

        // The original size is in the high 32 bits, so mask that to just get the compressed size
        size_t compressed_size = (embed.array_size & 0xFFFFFFFF);

        if (embed.filename.size())
        {
            code.Eol().Str("// ").Str(embed.filename);
            if (embed.array_size >> 32 > 0)
                code.Str(" (").itoa(embed.array_size >> 32).Str(" bytes)");
            else
                code.Str(" (uncompressed file)");
        }
        code.Eol();

        code.Str("const unsigned char ").Str(var_name);
        code.Str("[").itoa(compressed_size).Str("] {");
        source->writeLine(code);
        code.clear();
        // Since we don't call Eol() in the following loop, the indentation is not processed.
        code.Tab(2);

        size_t pos = 0;
        while (pos < compressed_size)
        {
            for (; pos < compressed_size && code.size() < cpp_line_length; ++pos)
            {
                code.itoa(embed.array_data[pos]) += ",";
            }
            if (pos >= compressed_size && code.GetCode().back() == ',')
            {
                code.GetCode().pop_back();
            }
            source->writeLine(code);
            code.clear();
            // Since we don't call Eol() in this loop, the indentation is not processed.
            code.Tab(2);
        }
        if (code.GetCode().back() == '\t')
        {
            code.pop_back();
        }
        code += "};\n";
        source->writeLine(code);
        code.clear();
    }
    if (processed_count > 0)
    {
        if (auto* frame = wxGetMainFrame(); frame)
        {
            frame->setStatusText(std::format("Completed writing {} data arrays", processed_count));
        }
    }
    source->writeLine();

    // Now add the functions

    for (auto& iter_array: m_embedded_data)
    {
        auto& var_name = iter_array.first;
        auto& embed = iter_array.second;
        if (embed.type == wxue::npos)
        {
            continue;
        }

        if (embed.array_size >> 32 > 0)
        {
            code.Str("std::string get_").Str(var_name) << "()\n{\n\t";
            // original size is in the high 32 bits
            code.Str("return std::string((const char*) get_data(");
            code.Str(var_name).Str(", sizeof(").Str(var_name);
            code << "), " << (embed.array_size >> 32) << ").get(), " << (embed.array_size >> 32)
                 << ");";
        }
        else
        {
            code.Str("std::pair<const unsigned char*, size_t> get_").Str(var_name) << "()";
            code.Eol().Str("{\n\t");
            code.Str("return std::make_pair(").Str(var_name).Str(", sizeof(").Str(var_name);
            code << "));";
        }

        code.Eol().Str("}\n");
        source->writeLine(code);
        code.clear();
    }

    if (code.size())
    {
        source->writeLine(code);
    }
}

auto DataHandler::WriteImagePostHeader(WriteCode* header) -> void
{
    bool is_namespace_written = false;

    std::vector<std::string> xml_function_list;
    std::vector<std::string> xml_function_filename_list;
    std::vector<std::string> data_function_list;
    std::vector<std::string> data_function_filename_list;

    for (auto& iter_array: m_embedded_data)
    {
        auto& var_name = iter_array.first;
        auto& embed = iter_array.second;
        if (embed.type == wxue::npos)
        {
            // filename was not found
            continue;
        }
        if (!is_namespace_written)
        {
            header->writeLine();
            header->writeLine("namespace wxue_data\n{");

            header->Indent();
            is_namespace_written = true;
        }
        if (embed.array_size >> 32 > 0)
        {
            xml_function_list.emplace_back(wxString() << "std::string get_" << var_name << "();");
            xml_function_filename_list.emplace_back(embed.filename);
        }
        else
        {
            data_function_list.emplace_back(
                wxString() << "std::pair<const unsigned char*, size_t> get_" << var_name << "();");
            data_function_filename_list.emplace_back(embed.filename);
        }
    }

    // Lambda to write function declarations with aligned comments
    auto write_function_list = [&](const std::vector<std::string>& func_list,
                                   const std::vector<std::string>& filename_list)
    {
        if (func_list.empty())
        {
            return;
        }

        size_t max_func_len = 0;
        for (const auto& func: func_list)
        {
            if (func.size() > max_func_len)
            {
                max_func_len = func.size();
            }
        }
        max_func_len += 2;  // Add two spaces before the comment

        for (size_t i = 0; i < func_list.size(); ++i)
        {
            const auto& func = func_list[i];
            const auto& filename = filename_list[i];
            if (filename.size())
            {
                header->writeLine(std::format(
                    "{}{}// {}", func, std::string(max_func_len - func.size(), ' '), filename));
            }
        }
        header->writeLine();
    };

    write_function_list(data_function_list, data_function_filename_list);
    write_function_list(xml_function_list, xml_function_filename_list);

    for (auto& iter_array: m_embedded_data)
    {
        auto& var_name = iter_array.first;
        auto& embed = iter_array.second;
        if (embed.type == wxue::npos)
        {
            // filename was not found
            continue;
        }
        if (!is_namespace_written)
        {
            header->writeLine();
            header->writeLine("namespace wxue_data\n{");

            header->Indent();
            is_namespace_written = true;
        }
        if (embed.filename.size())
        {
            header->writeLine(std::format("// {}", embed.filename));
        }
        if (embed.array_size >> 32 > 0 && Project.AddOptionalComments())
        {
            header->writeLine(
                std::format("extern const unsigned char {}[{}]; // Original size: {:L} bytes",
                            var_name, (embed.array_size & 0xFFFFFFFF), (embed.array_size >> 32)));
        }
        else
        {
            header->writeLine(std::format("extern const unsigned char {}[{}];", var_name,
                                          (embed.array_size & 0xFFFFFFFF)));
        }
    }

    if (is_namespace_written)
    {
        header->Unindent();
        header->writeLine("}\n");
    }
}

bool DataHandler::NeedsUtilityHeader() const
{
    for (auto& iter_array: m_embedded_data)
    {
        auto& embed = iter_array.second;
        if (embed.array_size >> 32 == 0)
        {
            return true;
        }
    }
    return false;
}

/////////////////////////////////  data_list namespace functions /////////////////////////////////

Node* data_list::FindDataList()
{
    Node* data_node = nullptr;
    if (Project.get_ChildCount() > 0)
    {
        if (Project.get_Child(0)->is_Gen(gen_Data))
        {
            data_node = Project.get_Child(0);
        }
        else
        {
            for (const auto& iter: Project.get_ChildNodePtrs())
            {
                if (iter->is_Gen(gen_Data))
                {
                    data_node = iter.get();
                    break;
                }
            }
        }
    }
    return data_node;
}
