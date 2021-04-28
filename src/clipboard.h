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

bool isClipboardDataAvailable();
NodeSharedPtr GetClipboardNode();

class wxUtf8DataObject : public wxDataObjectSimple
{
public:
    wxUtf8DataObject() : wxDataObjectSimple(wxDF_TEXT) {}

    bool SetData(size_t len, const void* buf) override;
    size_t GetDataSize() const override { return (m_text.size() + 1); }
    bool GetDataHere(void* buf) const override;

    // Note that this ia *not* a const return, so you can modify it if needed
    ttlib::cstr& GetText() { return m_text; }

private:
    ttlib::cstr m_text;
};

// We only use this class to indicate what format wxDF_TEXT is in.
class wxUEDataObject : public wxDataObjectSimple
{
public:
    wxUEDataObject() : wxDataObjectSimple(wxDataFormat(txt_OurClipboardFormat)) {}

    bool SetData(size_t len, const void* buf) override
    {
        if (len == sizeof(size_t))
            memcpy(&m_hash, buf, len);
        return true;
    }

    size_t GetDataSize() const override { return sizeof(size_t); }
    bool GetDataHere(void* buf) const override
    {
        memcpy(buf, &m_hash, sizeof(size_t));
        return true;
    }

    // Note that this ia *not* a const return, so you can modify it if needed
    size_t& GetHash() { return m_hash; }

private:
    size_t m_hash { 0 };
};

// Header-only class that automatically closes the clipboard in the destructor
class SmartClipboard
{
public:
    SmartClipboard() { m_is_opened = wxTheClipboard->Open(); }
    ~SmartClipboard() { wxTheClipboard->Close(); }

    bool IsOpened() { return m_is_opened; }

private:
    bool m_is_opened;
};
