/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame and derivative common generator functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

class Code;
class wxPropertyGridEvent;
class NodeProperty;
class Node;

namespace FrameCommon
{
    enum
    {
        frame_normal,
        frame_sdi_doc,
        frame_sdi_child,
        frame_mdi_doc,
        frame_mdi_child,
        frame_aui,
        frame_aui_child,
    };

    bool ConstructionCode(Code& code, int frame_type = frame_normal);
    bool SettingsCode(Code& code, int frame_type = frame_normal);
    bool AfterChildrenCode(Code& code, int frame_type = frame_normal);
    bool HeaderCode(Code& code, int frame_type = frame_normal);

    bool BaseClassNameCode(Code& code);
    bool AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node);

}  // namespace FrameCommon
