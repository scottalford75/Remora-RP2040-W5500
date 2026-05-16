#ifndef STEPGEN_H
#define STEPGEN_H

#include <cstdint>

#include "../extern.h"
#include "../../drivers/pin/pin.h"

#include "basic_stepgen.h"

using Stepgen = BasicStepgen<Pin>;

Stepgen *createStepgen(void);

#endif
