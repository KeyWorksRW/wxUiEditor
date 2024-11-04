#ifndef CLIPPATHELEMENT_H
#define CLIPPATHELEMENT_H

#include "graphicselement.h"

namespace wxlunasvg {

class LayoutClipPath;

class ClipPathElement final : public GraphicsElement {
public:
    ClipPathElement();

    Units clipPathUnits() const;
    std::unique_ptr<LayoutClipPath> getClipper(LayoutContext* context);
};

} // namespace wxlunasvg

#endif // CLIPPATHELEMENT_H
