#include <algorithm>
#include <iostream>
#include <regex>
#include <string>

#include "bodyID_utils.hpp"

using namespace CPPSpice;

bool is_valid_date_format(const std::string& input) {
   if (!std::regex_match(input, std::regex("[0-9]{4}-[A-Z]{3}-[0-9]{2}"))) {
      std::cout << "Error: input epoch does not match the required format." << std::endl;
      return false;
   }

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
   std::cout << occulterID << std::endl;

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