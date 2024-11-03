#include "graphicselement.h"
#include "parser.h"

namespace wxlunasvg {

GraphicsElement::GraphicsElement(ElementID id)
    : StyledElement(id)
{
}

Transform GraphicsElement::transform() const
{
    auto& value = get(PropertyID::Transform);
    return Parser::parseTransform(value);
}

} // namespace wxlunasvg
