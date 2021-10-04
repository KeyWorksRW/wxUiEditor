/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles reading and writing OS clipboard data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "clipboard.h"

#include "import/import_formblder.h"  // FormBuilder -- Import a wxFormBuider project
#include "import/import_wxsmith.h"    // WxSmith -- Import a wxSmith file

#include "mainframe.h"     // Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

#include "../../pugixml/pugixml.hpp"

bool isClipboardDataAvailable()
{
    if (wxTheClipboard->IsSupported(wxDataFormat(txt_OurClipboardFormat)))
        return true;
    else if (wxTheClipboard->IsSupported(wxDataFormat("wxFormBuilderDataFormat")))
        return true;
    else if (wxTheClipboard->IsSupported(wxDataFormat("wxSmith XML")))
        return true;
    else if (wxTheClipboard->IsSupported(wxDataFormat("DataObject")) && wxTheClipboard->IsSupported(wxDF_TEXT))
        return true;

    return false;
}

NodeSharedPtr GetClipboardNode()
{
    SmartClipboard clip;
    if (clip.IsOpened())
    {
        if (wxTheClipboard->IsSupported(wxDataFormat(txt_OurClipboardFormat)))
        {
            // Start by getting the stored hash number and comparing it with our internal clipboard hash number. If they are
            // the same, then assume nothing has changed and avoid the memory allocation and parsing and simply use the
            // internal clipboard.

            wxUEDataObject data;
            wxTheClipboard->GetData(data);
            if (wxGetFrame().GetClipHash() == data.GetHash())
                return {};
        }

        pugi::xml_document doc;
        pugi::xml_parse_result result;

        if (wxTheClipboard->IsSupported(wxDF_TEXT))
        {
            // We can't use wxTextDataObject on Windows because it will convert the text to UNICODE using the current locale.
            // The text we are retrieving is UTF8 and we don't need wxString to convert it to UTF16 since we use is directly.

            wxUtf8DataObject data;
            wxTheClipboard->GetData(data);
            result = doc.load_string(data.GetText().c_str());
        }

        if (!result)
        {
            appMsgBox("Unable to parse the object in the clipboard", "Paste Clipboard");
            return {};
        }

        auto root = doc.first_child();

        if (wxTheClipboard->IsSupported(wxDataFormat(txt_OurClipboardFormat)))
        {
            return g_NodeCreator.CreateNode(root);
        }
        else if (wxTheClipboard->IsSupported(wxDataFormat("wxFormBuilderDataFormat")))
        {
            FormBuilder fb;
            return fb.CreateFbpNode(root, nullptr);
        }
        else if (ttlib::is_sameas(root.name(), "resource", tt::CASE::either))
        {
            // wxSmith encloses the object with "<resource>"
            auto child = root.first_child();
            WxSmith smith;
            return smith.CreateXrcNode(child, nullptr);
        }

    }

    return {};
}

bool wxUtf8DataObject::SetData(size_t len, const void* buf)
{
    // We should never even come close to this amount. It's much more likely that a length this size is an attempt by malware
    // to crash any program attempting to paste clipboard data.
    if (len > (1024 * 1024 * 1024))
        return false;

    m_text.reserve(len + 1);
    memcpy(m_text.data(), buf, len);
    m_text.data()[len] = 0;
    return true;
}

bool wxUtf8DataObject::GetDataHere(void* buf) const
{
    memcpy(buf, m_text.data(), m_text.size() + 1);
    return true;
}
