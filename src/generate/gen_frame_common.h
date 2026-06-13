/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame and derivative common generator functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Code;
class wxPropertyGridEvent;
class NodeProperty;
class Node;

namespace FrameCommon
{
    enum class FrameType
    {
        frame_normal,
        frame_sdi_doc,
        frame_sdi_child,
        frame_mdi_doc,
        frame_mdi_child,
        frame_aui,
        frame_aui_child,
    };

    using enum FrameType;

    bool ConstructionCode(Code& code, FrameType frame_type = FrameType::frame_normal);
    bool SettingsCode(Code& code, FrameType frame_type = FrameType::frame_normal);
    bool AfterChildrenCode(Code& code, FrameType frame_type = FrameType::frame_normal);
    bool HeaderCode(Code& code, FrameType frame_type = FrameType::frame_normal);

    bool BaseClassNameCode(Code& code);
    bool AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node);
}  // namespace FrameCommon
