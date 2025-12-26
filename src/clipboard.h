/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles reading and writing OS clipboard data
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/clipbrd.h>  // wxClipboad class and clipboard functions

constexpr const char* txt_OurClipboardFormat = "wxUiEditor";

class Node;
using NodeSharedPtr = std::shared_ptr<Node>;

// Declared in clipboard.h. Returns true if the external clipboard condtains data that we can
// paste. This will either be from a different instance of wxUiEditor, or from wxFormBuilder
// or wxSmith.
[[nodiscard]] auto isClipboardDataAvailable() -> bool;

// Pass false as the parameter to prevent any error messages
[[nodiscard]] auto GetClipboardNode(bool warn_if_problems = true) -> NodeSharedPtr;

class wxUtf8DataObject : public wxDataObjectSimple
{
public:
    wxUtf8DataObject() : wxDataObjectSimple(wxDF_TEXT) {}

    auto SetData(size_t len, const void* buf) -> bool override;
    [[nodiscard]] auto GetDataSize() const -> size_t override { return (m_text.size() + 1); }
    [[nodiscard]] auto GetDataHere(void* buf) const -> bool override;

    // Note that this ia *not* a const return, so you can modify it if needed
    [[nodiscard]] auto GetText() -> wxue::string& { return m_text; }

private:
    wxue::string m_text;
};

// We only use this class to indicate what format wxDF_TEXT is in.
class wxUEDataObject : public wxDataObjectSimple
{
public:
    wxUEDataObject() : wxDataObjectSimple(wxDataFormat(txt_OurClipboardFormat)) {}

    auto SetData(size_t len, const void* buf) -> bool override
    {
        if (len == sizeof(size_t))
        {
            memcpy(&m_hash, buf, len);
        }
        return true;
    }

    [[nodiscard]] auto GetDataSize() const -> size_t override { return sizeof(size_t); }
    [[nodiscard]] auto GetDataHere(void* buf) const -> bool override
    {
        memcpy(buf, &m_hash, sizeof(size_t));
        return true;
    }

    // Note that this ia *not* a const return, so you can modify it if needed
    [[nodiscard]] auto GetHash() -> size_t& { return m_hash; }

private:
    size_t m_hash { 0 };
};

// Header-only class that automatically closes the clipboard in the destructor
class SmartClipboard
{
public:
    SmartClipboard() { m_is_opened = wxTheClipboard->Open(); }
    ~SmartClipboard() { wxTheClipboard->Close(); }

    [[nodiscard]] auto IsOpened() -> bool { return m_is_opened; }

private:
    bool m_is_opened;
};
