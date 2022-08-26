#pragma once

#include <string>

#include "includes_common.hpp"

namespace CPPSpice {
   SpiceCell* PerformOccultationSearch(
      const std::string& lower_bound_epoch,
      const std::string& upper_bound_epoch,
      const double       step_size,
      const std::string& occultation_type,
      const std::string& occulting_body,
      const std::string& occulting_body_shape,
      const std::string& occulting_body_frame,
      const std::string& target_body,
      const std::string& target_body_shape,
      const std::string& target_body_frame,
      const std::string& aberration_corrections,
      const std::string& observer_body,
      const double       tolerance);

   void ReportSummary(SpiceCell* result);
}