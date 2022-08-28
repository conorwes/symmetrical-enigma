#pragma once

#include <string>

#include "includes_common.hpp"

namespace CPPSpice {
   SpiceCell* PerformOccultationSearch(const SimulationData& data);

   void ReportSummary(SpiceCell* result);
}