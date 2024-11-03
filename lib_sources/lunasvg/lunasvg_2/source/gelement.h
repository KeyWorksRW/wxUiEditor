#ifndef GELEMENT_H
#define GELEMENT_H

#include "graphicselement.h"

namespace wxlunasvg {

class GElement final : public GraphicsElement {
public:
    GElement();

    void layout(LayoutContext* context, LayoutContainer* current) final;
};

} // namespace wxlunasvg

#endif // GELEMENT_H
