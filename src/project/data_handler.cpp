/////////////////////////////////////////////////////////////////////////////
// Purpose:   DataHandler class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if __has_include(<format>)
    #include <format>
#endif

#include "data_handler.h"

#include <wx/utils.h>  // For wxBusyCursor

#include <wx/filename.h>  // wxFileName - encapsulates a file path
#include <wx/mstream.h>   // Memory stream classes
#include <wx/wfstream.h>  // File stream classes
#include <wx/xml/xml.h>   // wxXmlDocument - XML parser & data holder class
#include <wx/zstream.h>   // zlib stream classes

#include <fstream>

#include <pugixml.hpp>

#include "code.h"             // Code -- Helper class for generating code
#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "project_handler.h"  // ProjectHandler class
#include "utils.h"            // Miscellaneous utility functions
#include "write_code.h"       // Write code to Scintilla or file

#include "../generate/gen_data_list.h"  // DataGenerator -- Data List generator

// Normally, wxMemoryInputStream inputStream, wxZlibOutputStream outputStream
bool CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream, size_t compressed_size);

DataHandler& ProjectData = DataHandler::getInstance();

void DataHandler::Initialize()
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

        std::set<tt_string> var_names;
        for (const auto& node: node_data_list->getChildNodePtrs())
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
            else
            {
                iter = m_embedded_data.begin();
            }
        }
    }

    auto rlambda = [&](Node* parent, auto&& rlambda) -> void
    {
        for (const auto& node: parent->getChildNodePtrs())
        {
            if (node->isGen(gen_data_folder))
            {
                rlambda(node.get(), rlambda);
                continue;
            }
            if (m_embedded_data.contains(node->as_string(prop_var_name)))
            {
                // If the filename is empty, there's nothing to load.
                if (node->as_string(prop_data_file).empty())
                    continue;

                auto& embed = m_embedded_data[node->as_string(prop_var_name)];

                if (embed.filename == node->as_string(prop_data_file) && embed.type != tt::npos)
                {
                    // If it's an XML file, then don't continue if xml_condensed has changed
                    if (!node->isGen(gen_data_xml) || node->as_bool(prop_xml_condensed_format) == embed.xml_condensed)
                        continue;
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

bool DataHandler::LoadAndCompress(Node* node)
{
    ASSERT(node->isGen(gen_data_string) || node->isGen(gen_data_xml));
    m_embedded_data[node->as_string(prop_var_name)] = {};
    auto& embed = m_embedded_data[node->as_string(prop_var_name)];
    embed.array_size = 0;
    embed.array_data = nullptr;
    embed.date_time = 0;
    embed.type = tt::npos;

    auto filename = node->as_string(prop_data_file);
    if (!filename.size())
    {
        embed.filename = "// No filename specified";
        return false;
    }

    auto [path, has_base_file] = Project.GetOutputPath(node->getParent(), GEN_LANG_CPLUSPLUS);
    if (has_base_file)
    {
        // true if the the base filename was returned, in which case we need to convert the
        // relative path to the output path to a relative path to the project path
        path.remove_filename();
        path.append(filename);
        filename = path;
        filename.make_absolute();
        filename.make_relative(Project.getProjectPath());
    }

    if (!filename.file_exists())
    {
        auto project_path = Project.getProjectPath();
        project_path.remove_filename();
        project_path.append(filename);
        if (!filename.file_exists())
        {
            return false;
        }
        else
        {
            embed.filename = project_path;
        }
    }
    else
    {
        embed.filename = filename;
    }
    embed.filename.backslashestoforward();

    if (node->isGen(gen_data_xml) && node->as_bool(prop_xml_condensed_format))
    {
        pugi::xml_document doc;
        if (auto result = doc.load_file_string(embed.filename, pugi::parse_trim_pcdata | pugi::parse_default); !result)
        {
            wxMessageDialog(wxGetMainFrame()->getWindow(), result.detailed_msg, "Parsing Error", wxOK | wxICON_ERROR)
                .ShowModal();
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
            // TODO: [KeyWorks - 03-16-2022] This would be really bad, though it should be impossible
            return false;
        }
        save_strem.Close();
        size_t org_size = (str.size() & 0xFFFFFFFF);
        size_t compressed_size = memory_stream.TellO();
        auto read_stream = memory_stream.GetOutputStreamBuffer();

        embed.type = 1;
        embed.xml_condensed = true;
        embed.array_size = (compressed_size | (org_size << 32));
        embed.array_data = std::make_unique<unsigned char[]>(compressed_size);
        memcpy(embed.array_data.get(), read_stream->GetBufferStart(), compressed_size);
        wxFileName wx_file(embed.filename);
        wx_file.GetTimes(nullptr, &embed.date_time, nullptr);
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
                // TODO: [KeyWorks - 03-16-2022] This would be really bad, though it should be impossible
                return false;
            }
            save_strem.Close();
            size_t org_size = (buffer.size() & 0xFFFFFFFF);
            size_t compressed_size = memory_stream.TellO();
            auto read_stream = memory_stream.GetOutputStreamBuffer();

            embed.type = node->isGen(gen_data_xml) ? 1 : 0;
            embed.xml_condensed = false;
            embed.array_size = (compressed_size | (org_size << 32));
            embed.array_data = std::make_unique<unsigned char[]>(compressed_size);
            memcpy(embed.array_data.get(), read_stream->GetBufferStart(), compressed_size);
            wxFileName wx_file(embed.filename);
            wx_file.GetTimes(nullptr, &embed.date_time, nullptr);
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
        embed.array_data = std::make_unique<unsigned char[]>(embed.array_size);
        stream.Read(embed.array_data.get(), embed.array_size);
        wxFileName wx_file(embed.filename);
        wx_file.GetTimes(nullptr, &embed.date_time, nullptr);
        return true;
    }

    return false;
}

void DataHandler::WriteDataConstruction(Code& code, WriteCode* source)
{
    // Make certain all files have been loaded
    wxBusyCursor wait;
    Initialize();

    for (const auto& node: code.node()->getChildNodePtrs())
    {
        if (m_embedded_data.contains(node->as_string(prop_var_name)))
        {
            auto& embed = m_embedded_data[node->as_string(prop_var_name)];

            // Since we've already called Initialize(), all valid files should have been
            // loaded, so if type is tt::npos then there's no file to load, or it can't be
            // loaded correctly.
            if (embed.type == tt::npos)
                continue;

            wxFileName wx_file(embed.filename);
            wxDateTime file_time;
            wx_file.GetTimes(nullptr, &file_time, nullptr);
            if (file_time == embed.date_time)
                continue;
            LoadAndCompress(node.get());
        }
    }

    ASSERT_MSG(code.is_cpp(), "This function is only used for C++ code generation");
    code.clear();
    // -12 to account for 8 indent + max 3 chars for number + comma
    size_t cpp_line_length = Project.as_size_t(prop_cpp_line_length) - 12;

    for (auto& iter_array: m_embedded_data)
    {
        auto& embed = iter_array.second;
        if (embed.type == tt::npos)
        {
            continue;
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

        code.Str("const unsigned char ").Str(iter_array.first);
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
    source->writeLine();

    // Now add the functions

    for (auto& iter_array: m_embedded_data)
    {
        auto& embed = iter_array.second;
        if (embed.type == tt::npos)
        {
            continue;
        }

        if (embed.array_size >> 32 > 0)
        {
            code.Str("std::string get_").Str(iter_array.first) << "()\n{\n\t";
            // original size is in the high 32 bits
            code.Str("return std::string((const char*) get_data(");
            code.Str(iter_array.first).Str(", sizeof(").Str(iter_array.first);
            code << "), " << (embed.array_size >> 32) << ").get(), " << (embed.array_size >> 32) << ");";
        }
        else
        {
            code.Str("std::pair<const unsigned char*, size_t> get_").Str(iter_array.first) << "()";
            code.Eol().Str("{\n\t");
            code.Str("return std::make_pair(").Str(iter_array.first).Str(", sizeof(").Str(iter_array.first);
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

void DataHandler::WriteImagePostHeader(WriteCode* header)
{
    bool is_namespace_written = false;
    for (auto& iter_array: m_embedded_data)
    {
        auto& embed = iter_array.second;
        if (embed.type == tt::npos)
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
            header->writeLine(tt_string("std::string get_") << iter_array.first << "();");
        }
        else
        {
            header->writeLine(tt_string("std::pair<const unsigned char*, size_t> get_") << iter_array.first << "();");
        }
    }
    header->writeLine();

    for (auto& iter_array: m_embedded_data)
    {
        auto& embed = iter_array.second;
        if (embed.type == tt::npos)
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
            header->writeLine(tt_string("// ") << embed.filename);
        }
        if (embed.array_size >> 32 > 0)
        {
            header->writeLine(tt_string("extern const unsigned char ")
                              << iter_array.first << '[' << (embed.array_size & 0xFFFFFFFF) << "]; // "
                              << (embed.array_size >> 32));
        }
        else
        {
            header->writeLine(tt_string("extern const unsigned char ")
                              << iter_array.first << '[' << (embed.array_size & 0xFFFFFFFF) << "];");
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

// clang-format off

inline constexpr const auto txt_get_data_function = R"===(
    // Convert compressed data string into a char array
    std::unique_ptr<unsigned char[]> get_data(const unsigned char* data,
        size_t size_data, size_t size_data_uncompressed)
    {
        auto str = std::unique_ptr<unsigned char[]>(new unsigned char[size_data_uncompressed]);
        wxMemoryInputStream stream_in(data, size_data);
        wxZlibInputStream zlib_strm(stream_in);
        zlib_strm.Read(str.get(), size_data_uncompressed);
        return str;
    };
)===";

// clang-format on

void BaseCodeGenerator::GenerateDataForm()
{
    ASSERT_MSG(m_form_node, "Attempting to generate Data List when no form was located.");

    if (!m_form_node->getChildCount())
    {
        return;
    }
    if (m_panel_type != HDR_PANEL)
    {
        m_source->writeLine("#include <wx/mstream.h>  // memory stream classes", indent::none);
        m_source->writeLine("#include <wx/zstream.h>  // zlib stream classes", indent::none);

        m_source->writeLine();
        m_source->writeLine("namespace wxue_data\n{");
        m_source->Indent();
        m_source->SetLastLineBlank();

        tt_string_vector function;
        function.ReadString(txt_get_data_function);
        for (auto& iter: function)
        {
            m_source->writeLine(iter, indent::none);
        }

        Code code(m_form_node, m_language);

        ProjectData.WriteDataConstruction(code, m_source);

        m_source->Unindent();
        m_source->writeLine("}\n");
    }

    /////////////// Header code ///////////////

    if (m_panel_type != CPP_PANEL)
    {
        m_header->writeLine();
        m_header->writeLine("#include <memory>  // for std::make_unique", indent::none);

        if (ProjectData.NeedsUtilityHeader())
        {
            m_header->writeLine("#include <utility>  // for std::pair", indent::none);
        }

        m_header->writeLine();
        m_header->writeLine("namespace wxue_data\n{");
        m_header->Indent();
        m_header->SetLastLineBlank();
        m_header->writeLine("std::unique_ptr<unsigned char[]> get_data(const unsigned char* data, size_t size_data, size_t "
                            "size_data_uncompressed);");

        m_header->writeLine();

        m_header->Unindent();
        m_header->writeLine("}\n");
    }
    ProjectData.WriteImagePostHeader(m_header);
}

//////////////////////////////////////////  data_list namespace functions  ////////////////////////////////

Node* data_list::FindDataList()
{
    Node* data_node = nullptr;
    if (Project.getChildCount() > 0)
    {
        if (Project.getChild(0)->isGen(gen_Data))
        {
            data_node = Project.getChild(0);
        }
        else
        {
            for (const auto& iter: Project.getChildNodePtrs())
            {
                if (iter->isGen(gen_Data))
                {
                    data_node = iter.get();
                    break;
                }
            }
        }
    }
    return data_node;
}
