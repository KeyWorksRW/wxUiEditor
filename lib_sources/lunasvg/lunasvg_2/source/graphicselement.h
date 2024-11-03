#ifndef GRAPHICSELEMENT_H
#define GRAPHICSELEMENT_H

#include "styledelement.h"

namespace wxlunasvg {

class GraphicsElement : public StyledElement {
public:
    GraphicsElement(ElementID id);

    Transform transform() const;
};

} // namespace wxlunasvg

#endif // GRAPHICSELEMENT_H
