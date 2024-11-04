#ifndef MASKELEMENT_H
#define MASKELEMENT_H

#include "styledelement.h"

namespace wxlunasvg {

class LayoutMask;

class MaskElement final : public StyledElement {
public:
    MaskElement();

    Length x() const;
    Length y() const;
    Length width() const;
    Length height() const;
    Units maskUnits() const;
    Units maskContentUnits() const;
    std::unique_ptr<LayoutMask> getMasker(LayoutContext* context);
};

} // namespace wxlunasvg

#endif // MASKELEMENT_H
