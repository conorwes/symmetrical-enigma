#pragma once

#include <iostream>
#include <string>
#include <tuple>

extern "C" {
#include "SpiceZpl.h"
#include "SpiceZdf.h"
#include "SpiceErr.h"
#include "SpiceEK.h"
#include "SpiceFrm.h"
#include "SpiceCel.h"
#include "SpiceCK.h"
#include "SpiceSPK.h"
#include "SpiceGF.h"
#include "SpiceOccult.h"
#include "SpiceZpr.h"
#include "SpiceZim.h"
}

namespace CPPSpice {
   using ParticipantDetails = std::tuple<std::string, std::string, std::string>;

   struct SimulationData {
      std::string        LowerBoundEpoch;
      std::string        UpperBoundEpoch;
      double             StepSize;
      std::string        OccultationType;
      ParticipantDetails OcculterDetails;
      ParticipantDetails TargetDetails;
      std::string        ObserverName;
      double             Tolerance;
   };
}   // namespace CPPSpice