#include <algorithm>
#include <iostream>
#include <regex>
#include <string>

#include <SpiceUsr.h>

bool is_valid_date_format(const std::string& input) {
   if (!std::regex_match(input, std::regex("[0-9]{4}-[A-Z]{3}-[0-9]{2}"))) {
      std::cout << "Error: input epoch does not match the required format." << std::endl;
      return false;
   }

   return true;
}

bool PerformQuery(
   const std::string& observerName,
   const std::string& targetName,
   const std::string& occulterName,
   const std::string& lowerEpoch,
   const std::string& upperEpoch) {
   // summarize our objective
   std::cout << "Searching for occultations in the '" << observerName << "-"
             << occulterName << "-" << targetName
             << "' system in the time window between " << lowerEpoch << " and "
             << upperEpoch << "." << std::endl;

   SpiceBoolean bail;
   SpiceBoolean rpt;
   SpiceChar*   win0;
   SpiceChar*   win1;
   SpiceChar    begstr[41];
   SpiceChar    endstr[41];

   SPICEDOUBLE_CELL(cnfine, 200);
   SPICEDOUBLE_CELL(result, 200);

   SpiceDouble et0;
   SpiceDouble et1;
   SpiceDouble left;
   SpiceDouble right;

   SpiceInt i;

   furnsh_c("gfocce_ex1.tm");

   win0 = "2030 JAN 01 00:00:00 TDB";
   win1 = "2040 JAN 01 00:00:00 TDB";
   str2et_c(win0, &et0);
   str2et_c(win1, &et1);

   wninsd_c(et0, et1, &cnfine);

   gfsstp_c(20.0);

   bail = SPICETRUE;
   rpt  = SPICETRUE;

   gfocce_c(
      "ANY",
      "MOON",
      "ellipsoid",
      "IAU_MOON",
      "SUN",
      "ellipsoid",
      "IAU_SUN",
      "LT",
      "EARTH",
      1.e-6,
      gfstep_c,
      gfrefn_c,
      rpt,
      gfrepi_c,
      gfrepu_c,
      gfrepf_c,
      bail,
      gfbail_c,
      &cnfine,
      &result);

   if (gfbail_c()) {
      /* Clear the CSPICE interrupt indication. This is an essential step for programs
       * that continue running after an interrupt; gfbail_c will continue to return
       * SPICETRUE until this step has been performed.*/
      gfclrh_c();

      /*We've trapped an interrupt signal. In a realistic application, the program would
       * continue operation from this point. In this simple example, we simply display a
       * message and quit.*/
      std::cout << "\nSearch was interrupted.\n\nThis message "
                   "was written after an interrupt signal\n"
                   "was trapped. By default, the program "
                   "would have terminated \nbefore this message "
                   "could be written.\n\n";
   }
   else {

      if (wncard_c(&result) == 0) {
         std::cout << "No occultation was found.\n";
      }
      else {
         for (i = 0; i < wncard_c(&result); i++) {
            /*fetch and display each occultation interval.*/
            wnfetd_c(&result, i, &left, &right);

            timout_c(left, "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)", 41, begstr);
            timout_c(right, "YYYY MON DD HR:MN:SC.###### ::TDB (TDB)", 41, endstr);

            std::cout << "Interval %d\n", (int)i;
            std::cout << "   Start time: %s\n", begstr;
            std::cout << "   Stop time:  %s\n", endstr;
         }
      }
   }

   /*auto          oName = static_cast<ConstSpiceChar*>(observerName.c_str());
   SpiceInt*     oID(0);
   SpiceBoolean* oFound(false);
   bodn2c_c(oName, oID, oFound);
   if (!oFound) {
      std::cout << "Unable to find the NAIF ID corresponding to the specified observer."
                << std::endl;
      return false;
   }
   else {
      std::cout << "The NAIF ID for the specified observer is: " << oID << std::endl;
   }*/

   return true;
}

bool are_valid_dates(const std::string& lower_bound, const std::string& upper_bound) {
   if (lower_bound == upper_bound) {
      std::cout << "Error: lower and upper bounds are identical." << std::endl;
      return false;
   }

   // TODO - add in some epoch checking, namely:
   // 1. upper > lower
   // 2. month is a real month
   // 3. day is a real day
   // 4. ???

   return true;
}

int main() {
   // first, let's retrieve the bodies involved
   std::cout << "Observer Name: ";
   std::string observerName = "";
   std::cin >> observerName;
   std::transform(
      observerName.begin(), observerName.end(), observerName.begin(), ::toupper);

   std::cout << "Target Name: ";
   std::string targetName = "";
   std::cin >> targetName;
   std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::toupper);

   std::cout << "Occulter Name: ";
   std::string occulterName = "";
   std::cin >> occulterName;
   std::transform(
      occulterName.begin(), occulterName.end(), occulterName.begin(), ::toupper);

   // next, let's retrieve the epochs which define our bounds
   std::cout << "Lower Bound Epoch (YYYY-MMM-DD): ";
   std::string input = "";
   std::cin >> input;
   if (!is_valid_date_format(input)) {
      return 1;
   }
   std::string startEpoch = input;

   std::cout << "Upper Bound Epoch (YYYY-MMM-DD): ";
   input = "";
   std::cin >> input;
   if (!is_valid_date_format(input)) {
      return 1;
   }
   std::string endEpoch = input;

   if (!are_valid_dates(startEpoch, endEpoch)) {
      return 1;
   }

   // execute the query:
   return PerformQuery(observerName, targetName, occulterName, startEpoch, endEpoch);

   return 0;
}