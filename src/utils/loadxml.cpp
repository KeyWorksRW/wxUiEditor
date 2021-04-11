/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load internal xml files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "../pugixml/pugixml.hpp"

#include "utils.h"  // Utility functions that work with properties

// #include "../xml/aui.hgz"         // aui_xml_gz[] == xml/aui.xml

#include "../xml/bars.hgz"        // bars_xml_gz[] == xml/bars.xml
#include "../xml/containers.hgz"  // containers_xml_gz[] == containers.xml
#include "../xml/forms.hgz"       // forms_xml_gz[] == xml/forms.xml
#include "../xml/interface.hgz"   // interface_xml_gz[] == xml/project.xml
#include "../xml/sizers.hgz"      // sizers_xml_gz[] == sizers.xml
#include "../xml/widgets.hgz"     // widgets_xml_gz[] == widgets.xml

struct GZIP_PAIR
{
    const char* filename;
    const unsigned char* gzip_data;
    size_t gzip_size;
};

// clang-format off
static const GZIP_PAIR gzip_pairs[] = {

    // { "aui", aui_xml_gz, sizeof(aui_xml_gz) },

    { "bars", bars_xml_gz, sizeof(bars_xml_gz) },
    { "containers", containers_xml_gz, sizeof(containers_xml_gz) },
    { "forms", forms_xml_gz, sizeof(forms_xml_gz) },
    { "interface", interface_xml_gz, sizeof(interface_xml_gz) },
    { "sizers", sizers_xml_gz, sizeof(sizers_xml_gz) },
    { "widgets", widgets_xml_gz, sizeof(widgets_xml_gz) },

};
// clang-format on

bool LoadInternalXmlDocFile(ttlib::cview file, pugi::xml_document& doc)
{
    for (auto& iter: gzip_pairs)
    {
        if (file.is_sameas(iter.filename))
        {
            auto str = LoadGzipString(iter.gzip_data, iter.gzip_size);
            if (str.size())
            {
                if (auto result = doc.load_string(str.c_str()); result)
                {
                    return true;
                }
                else
                {
                    FAIL_MSG(ttlib::cstr("xml/") << file << " is corrupted!");
                    return false;
                }
            }
        }
    }

    FAIL_MSG(ttlib::cstr("Unable to locate internal file: ") << file);
    return false;
}
