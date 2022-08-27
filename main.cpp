#include "support_utils.hpp"
#include "occultation_utils.hpp"

using namespace CPPSpice;

int main() {

   // Define some helpful lambdas for this program
   // A simple lambda to prompt the user to specify a kernel path.
   auto FurnishSPICEKernel = [](const std::string& kernel_name) -> void {
      std::string path;
      std::cout << "Please specify your " << kernel_name
                << " kernel's path: " << std::endl;
      std::getline(std::cin, path);

      // TODO - add some path validation here

      // Call the CSPICE furnsh_c routine
      furnsh_c(path.c_str());
      return;
   };

   // Before we do anything else, let's furnish the kernels we'll need for this program.
   // For this program, we'll need a pck, tsp, and bsp file, so furnish those now
   FurnishSPICEKernel("P Constants");
   FurnishSPICEKernel("Timespan");
   FurnishSPICEKernel("Planetary Ephemerides");

   // Now we're ready to roll. Let's get the other things we'll need here.

   // Let's get the date range and perform some validation
   std::cout << "Lower Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::string input;
   std::getline(std::cin, input);
   if (!IsValidDateFormat(input)) {
      return 1;
   }
   std::string startEpoch = input;

   std::cout << "Upper Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::getline(std::cin, input);
   if (!IsValidDateFormat(input)) {
      return 1;
   }
   std::string endEpoch = input;

   if (!AreDateBoundsValid(startEpoch, endEpoch)) {
      return 1;
   }

   auto results = CPPSpice::PerformOccultationSearch(
      startEpoch,
      endEpoch,
      double(20.0),
      "ANY",
      "MOON",
      "ellipsoid",
      "IAU_MOON",
      "SUN",
      "ellipsoid",
      "IAU_SUN",
      "LT",
      "EARTH",
      1e-6);

   CPPSpice::ReportSummary(std::move(results));

   // first, let's retrieve the bodies involved
   /*std::cout << "Observer Name: ";
   std::string observerName = "";
   std::cin >> observerName;
   std::transform(
      observerName.begin(), observerName.end(), observerName.begin(), ::toupper);

   auto observerID = GetNAIFIDfromName(observerName);
   std::cout << observerID << std::endl;

   std::cout << "Target Name: ";
   std::string targetName = "";
   std::cin >> targetName;
   std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::toupper);

   auto targetID = GetNAIFIDfromName(targetName);
   std::cout << targetID << std::endl;

   std::cout << "Occulter Name: ";
   std::string occulterName = "";
   std::cin >> occulterName;
   std::transform(
      occulterName.begin(), occulterName.end(), occulterName.begin(), ::toupper);

   auto occulterID = GetNAIFIDfromName(occulterName);
   std::cout << occulterID << std::endl;*/

   // next, let's retrieve the epochs which define our bounds
   /*std::cout << "Lower Bound Epoch (YYYY-MMM-DD): ";
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
   }*/

   return 0;
}