#ifndef STOPELEMENT_H
#define STOPELEMENT_H

#include "styledelement.h"

namespace wxlunasvg {

class StopElement final : public StyledElement {
public:
    StopElement();

    double offset() const;
    Color stopColorWithOpacity() const;
};

} // namespace wxlunasvg

#endif // STOPELEMENT_H
