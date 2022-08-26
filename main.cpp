#include "support_utils.hpp"
#include "occultation_utils.hpp"

using namespace CPPSpice;

int main() {

   auto results = CPPSpice::PerformOccultationSearch(
      "2030 JAN 01 00:00:00 TDB",
      "2040 JAN 01 00:00:00 TDB",
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