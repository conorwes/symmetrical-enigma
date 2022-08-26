#include <algorithm>
#include <regex>

#include "support_utils.hpp"
#include "includes_common.hpp"

int CPPSpice::GetNAIFIDfromName(const std::string& name) {
   SpiceInt     code(0);
   SpiceBoolean found(false);
   bodn2c_c(name.c_str(), &code, &found);

   if (found) {
      return code;
   }
   else {
      std::cout << "Error: couldn't find an NAIF ID for the specified object '" << name
                << "'." << std::endl;
      return -1;
   }
}

bool CPPSpice::IsValidDateFormat(const std::string& input) {
   if (!std::regex_match(input, std::regex("[0-9]{4}-[A-Z]{3}-[0-9]{2}"))) {
      std::cout << "Error: input epoch does not match the required format." << std::endl;
      return false;
   }

   return true;
}

bool CPPSpice::AreDateBoundsValid(
   const std::string& lower_bound, const std::string& upper_bound) {
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