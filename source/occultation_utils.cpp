#include "occultation_utils.hpp"

SpiceCell* CPPSpice::PerformOccultationSearch(const SimulationData& data) {

   double et0;
   double et1;
   str2et_c(data.LowerBoundEpoch.c_str(), &et0);
   str2et_c(data.UpperBoundEpoch.c_str(), &et1);

   SPICEDOUBLE_CELL(cnfine, 200);
   SPICEDOUBLE_CELL(result, 200);
   wninsd_c(et0, et1, &cnfine);

   gfsstp_c(data.StepSize);

   gfocce_c(
      data.OccultationType.c_str(),
      std::get<0>(data.OcculterDetails).c_str(),
      std::get<1>(data.OcculterDetails).c_str(),
      std::get<2>(data.OcculterDetails).c_str(),
      std::get<0>(data.TargetDetails).c_str(),
      std::get<1>(data.TargetDetails).c_str(),
      std::get<2>(data.TargetDetails).c_str(),
      "LT",
      data.ObserverName.c_str(),
      data.Tolerance,
      gfstep_c,
      gfrefn_c,
      true,
      gfrepi_c,
      gfrepu_c,
      gfrepf_c,
      true,
      gfbail_c,
      &cnfine,
      &result);

   return &result;
}

void CPPSpice::ReportSummary(SpiceCell* result) {
   SpiceInt    i;
   SpiceDouble left;
   SpiceDouble right;
   SpiceChar   begstr[41];
   SpiceChar   endstr[41];

   if (gfbail_c()) {
      /*
      Clear the CSPICE interrupt indication. This is
      an essential step for programs that continue
      running after an interrupt; gfbail_c will
      continue to return SPICETRUE until this step
      has been performed.
      */
      gfclrh_c();

      /*
      We've trapped an interrupt signal. In a realistic
      application, the program would continue operation
      from this point. In this simple example, we simply
      display a message and quit.
      */
      std::cout << "Error: Search was interrupted." << std::endl;
   }
   else {

      if (wncard_c(result) == 0) {
         printf("No occultation was found.\n");
      }
      else {
         for (i = 0; i < wncard_c(result); i++) {
            /*
            fetch and display each occultation interval.
            */
            wnfetd_c(result, i, &left, &right);

            timout_c(left, "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)", 41, begstr);
            timout_c(right, "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)", 41, endstr);

            std::cout << "Interval " << i << std::endl;
            std::cout << "   Start time: " << begstr << std::endl;
            std::cout << "   Stop time:  " << endstr << std::endl;
         }
      }
   }
}