/////////////////////////////////////////////////////////////////////////////
// Purpose:   Render a class inheritance graph as a standalone SVG document.
//            Self-contained layered-DAG layout — no graphviz/external tools,
//            no wxWidgets dependency. Reusable from both the parser pipeline
//            (generate at parse time) and the viewer (generate on the fly).
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <span>
#include <string>
#include <vector>

namespace docparser
{

    // One class in an inheritance graph.
    struct InheritGraphNode
    {
        std::string name;                // display text and unique key
        std::string url;                 // link target (empty -> not clickable)
        std::string tooltip;             // hover description (optional)
        std::vector<std::string> bases;  // names of DIRECT base classes
        bool highlight = false;          // true for the focus class (shaded box)
    };

    // Render the supplied nodes as a layered inheritance graph and return a
    // complete SVG document. When 'horizontal' is false (default), base classes
    // appear at the top and derived classes below (ranks are rows). When
    // 'horizontal' is true, base classes appear on the left and derived classes
    // on the right (ranks are columns) — recommended for graphs where a single
    // class fans out to many direct children. Layering, ordering and positioning
    // are all computed internally.
    //
    // A 'bases' entry that names a class not present in 'nodes' is ignored (its
    // edge is skipped); names are matched exactly and must be unique. Returns an
    // empty string when 'nodes' is empty. When non-null, 'out_width'/'out_height'
    // receive the document's natural pixel size (useful for rasterizing).
    [[nodiscard]] std::string RenderInheritanceSvg(std::span<const InheritGraphNode> nodes,
                                                   double* out_width = nullptr,
                                                   double* out_height = nullptr,
                                                   bool horizontal = false);

}  // namespace docparser
