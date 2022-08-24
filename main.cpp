#include <iostream>
#include <regex>
#include <string>

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

   std::cout << "Target Name: ";
   std::string targetName = "";
   std::cin >> targetName;

   std::cout << "Occulter Name: ";
   std::string occulterName = "";
   std::cin >> occulterName;

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

   std::cout << "Searching for occultations in the " << observerName << "-"
             << occulterName << "-" << targetName << " system in the range between "
             << startEpoch << " and " << endEpoch << "." << std::endl;
   return 0;
}