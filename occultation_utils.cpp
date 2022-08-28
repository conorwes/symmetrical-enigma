#include "occultation_utils.hpp"

SpiceCell* CPPSpice::PerformOccultationSearch(const SimulationData& data) {

   SpiceBoolean bail;
   SpiceBoolean rpt;

   SPICEDOUBLE_CELL(cnfine, 200);
   SPICEDOUBLE_CELL(result, 200);

   SpiceDouble et0;
   SpiceDouble et1;

   str2et_c(data.LowerBoundEpoch.c_str(), &et0);
   str2et_c(data.UpperBoundEpoch.c_str(), &et1);

   wninsd_c(et0, et1, &cnfine);

   gfsstp_c(data.StepSize);

   bail = SPICETRUE;
   rpt  = SPICETRUE;

   gfocce_c(
      /*ConstSpiceChar* occtyp*/ data.OccultationType.c_str(),
      /*ConstSpiceChar* front*/ std::get<0>(data.OcculterDetails).c_str(),
      /*ConstSpiceChar *fshape*/ std::get<1>(data.OcculterDetails).c_str(),
      /*ConstSpiceChar *fframe*/ std::get<2>(data.OcculterDetails).c_str(),
      /*ConstSpiceChar* back*/ std::get<0>(data.TargetDetails).c_str(),
      /*ConstSpiceChar* bshape*/ std::get<1>(data.TargetDetails).c_str(),
      /*ConstSpiceChar* bframe*/ std::get<2>(data.TargetDetails).c_str(),
      /*ConstSpiceChar* abcorr*/ "LT",
      /*ConstSpiceChar* obsrvr*/ data.ObserverName.c_str(),
      /*SpiceDouble tol*/ data.Tolerance,
      /*void (*udstep)(SpiceDouble et, SpiceDouble *step)*/ gfstep_c,
      /*void (*udrefn)(SpiceDouble t1, SpiceDouble t2, SpiceBoolean s1, SpiceBoolean s2,
         SpiceDouble *t)*/
      gfrefn_c,
      /*SpiceBoolean rpt*/ rpt,
      /*void (*udrepi)(SpiceCell *cnfine, ConstSpiceChar *srcpre, ConstSpiceChar
       *srcsuf)*/
      gfrepi_c,
      /*void (*udrepu)(SpiceDouble ivbeg, SpiceDouble ivend, SpiceDouble et)*/ gfrepu_c,
      /*void (*udrepf)()*/ gfrepf_c,
      /*SpiceBoolean bail*/ bail,
      /*SpiceBoolean (*udbail)()*/ gfbail_c,
      /*SpiceCell *cnfine*/ &cnfine,
      /*SpiceCell *result*/ &result);

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
      printf(
         "\nSearch was interrupted.\n\nThis message "
         "was written after an interrupt signal\n"
         "was trapped. By default, the program "
         "would have terminated \nbefore this message "
         "could be written.\n\n");
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