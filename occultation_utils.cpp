#include "occultation_utils.hpp"

SpiceCell* CPPSpice::PerformOccultationSearch(
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
   const double       tolerance) {

   SpiceBoolean bail;
   SpiceBoolean rpt;

   SpiceChar* win0;
   SpiceChar* win1;

   SPICEDOUBLE_CELL(cnfine, 200);
   SPICEDOUBLE_CELL(result, 200);

   SpiceDouble et0;
   SpiceDouble et1;

   win0 = "2030 JAN 01 00:00:00 TDB";
   win1 = "2040 JAN 01 00:00:00 TDB";

   str2et_c(win0, &et0);
   str2et_c(win1, &et1);

   wninsd_c(et0, et1, &cnfine);

   gfsstp_c(20.0);

   bail = SPICETRUE;
   rpt  = SPICETRUE;

   gfocce_c(
      /*ConstSpiceChar* occtyp*/ occultation_type.c_str(),
      /*ConstSpiceChar* front*/ occulting_body.c_str(),
      /*ConstSpiceChar *fshape*/ occulting_body_shape.c_str(),
      /*ConstSpiceChar *fframe*/ occulting_body_frame.c_str(),
      /*ConstSpiceChar* back*/ target_body.c_str(),
      /*ConstSpiceChar* bshape*/ target_body_shape.c_str(),
      /*ConstSpiceChar* bframe*/ target_body_frame.c_str(),
      /*ConstSpiceChar* abcorr*/ aberration_corrections.c_str(),
      /*ConstSpiceChar* obsrvr*/ observer_body.c_str(),
      /*SpiceDouble tol*/ tolerance,
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