#include "occultation_utils.hpp"
#include <iostream>

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

void CPPSpice::TODO_GFFOCE_WRAPPER() {

   SpiceBoolean bail;
   SpiceBoolean rpt;

   SpiceChar* win0;
   SpiceChar* win1;
   SpiceChar  begstr[41];
   SpiceChar  endstr[41];

   SPICEDOUBLE_CELL(cnfine, 200);
   SPICEDOUBLE_CELL(result, 200);

   SpiceDouble et0;
   SpiceDouble et1;
   SpiceDouble left;
   SpiceDouble right;

   SpiceInt i;

   furnsh_c("D:/Repositories/symmetrical-enigma/include/pck.tpc");

   furnsh_c("D:/Repositories/symmetrical-enigma/include/naif0012.tls");
   furnsh_c("D:/Repositories/symmetrical-enigma/include/de421.bsp");
   furnsh_c("D:/Repositories/symmetrical-enigma/include/pck00010.tpc");

   win0 = "2030 JAN 01 00:00:00 TDB";
   win1 = "2040 JAN 01 00:00:00 TDB";

   str2et_c(win0, &et0);
   str2et_c(win1, &et1);

   wninsd_c(et0, et1, &cnfine);

   gfsstp_c(20.0);

   bail = SPICETRUE;
   rpt  = SPICETRUE;

   gfocce_c(
      /*ConstSpiceChar* occtyp*/ "ANY",
      /*ConstSpiceChar* front*/ "MOON",
      /*ConstSpiceChar *fshape*/ "ellipsoid",
      /*ConstSpiceChar *fframe*/ "IAU_MOON",
      /*ConstSpiceChar* back*/ "SUN",
      /*ConstSpiceChar* bshape*/ "ellipsoid",
      /*ConstSpiceChar* bframe*/ "IAU_SUN",
      /*ConstSpiceChar* abcorr*/ "LT",
      /*ConstSpiceChar* obsrvr*/ "EARTH",
      /*SpiceDouble tol*/ 1.e-6,
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

      if (wncard_c(&result) == 0) {
         printf("No occultation was found.\n");
      }
      else {
         for (i = 0; i < wncard_c(&result); i++) {
            /*
            fetch and display each occultation interval.
            */
            wnfetd_c(&result, i, &left, &right);

            timout_c(left, "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)", 41, begstr);
            timout_c(right, "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)", 41, endstr);

            std::cout << "Interval " << i << std::endl;
            std::cout << "   Start time: " << begstr << std::endl;
            std::cout << "   Stop time:  " << endstr << std::endl;
         }
      }
   }
}