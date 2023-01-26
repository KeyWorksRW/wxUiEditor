/////////////////////////////////////////////////////////////////////////////
// Purpose:   Import a wxSmith file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node_classes.h"

#include "import_xml.h"  // ImportXML -- Base class for XML importing

class WxSmith : public ImportXML
{
public:
    WxSmith();

    bool Import(const tt_wxString& filename, bool write_doc = true) override;
};
