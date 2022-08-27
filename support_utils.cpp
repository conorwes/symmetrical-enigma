#include <algorithm>

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

// TODO - expand capabilities to allow for arbitrary formats
bool CPPSpice::IsValidDate(const std::string& input) {

   // Define some lambdas for clarity
   // simple lambda to check for leap year. have it as a lambda for cleanliness later
   auto is_leap_year = [](int year) {
      if (year % 400 == 0) {
         return true;
      }
      else if (year % 100 == 0) {
         return false;
      }
      else if (year % 4 == 0) {
         return true;
      }

      return false;
   };

   // simple lambda to check individual components of a time (i.e. hour:minute:second) for
   // compliance with range
   auto is_within_time_range =
      [](const std::string& input, const int offset, const std::string& time_unit) {
         auto string = input.substr(offset, 2);
         auto number = std::atoi(string.c_str());

         int lower_bound{0};
         int upper_bound{0};

         if (time_unit == "hour") {
            upper_bound = 23;
         }
         else {
            upper_bound = 59;
         }

         if (number < lower_bound || number > upper_bound) {
            std::cout << "Error: input " << time_unit << " '" << string
                      << "' is not valid." << std::endl;
            return false;
         };

         return true;
      };

   // First we need to check that the input matches the expected string format
   if (!std::regex_match(input, date_format)) {
      std::cout << "Error: input epoch << '" << input
                << "' does not match the required format." << std::endl;
      return false;
   }

   // Now we can perform some validation, including:
   // 1. month is a real month
   auto month_string = input.substr(5, 3);
   auto it           = std::find_if(
      valid_months.begin(),
      valid_months.end(),
      [&month_string](const std::pair<std::string, int>& element) {
         return element.first == month_string;
      });

   if (it == valid_months.end()) {
      std::cout << "Error: input month '" << month_string
                << "' does not correspond to a valid month." << std::endl;
      return false;
   }

   // 2. day is a real day of said month
   auto day_string = input.substr(9, 2);
   auto day_number = std::atoi(day_string.c_str());

   // add support for leap years
   auto max_day = it->second;
   if (month_string == "FEB" && is_leap_year(std::atoi(input.substr(0, 4).c_str()))) {
      max_day++;
   }

   if (day_number < 1 || day_number > max_day) {
      std::cout << "Error: input day '" << day_string
                << "' does not correspond to a valid day number for the month of '"
                << month_string << "'." << std::endl;
      return false;
   }

   // 3. time is 00-23, 00-59, 00-59
   if (!is_within_time_range(input, 12, "hour")) {
      return false;
   }

   if (!is_within_time_range(input, 15, "minute")) {
      return false;
   }

   if (!is_within_time_range(input, 18, "second")) {
      return false;
   }

   // 4. year is covered by the bsp we've furnished
   SpiceDouble input_double{0.0};
   SpiceDouble pos_vel[6];
   SpiceDouble lt;
   str2et_c(input.c_str(), &input_double);

   SpiceInt     target_id;
   SpiceInt     observer_id;
   SpiceBoolean found;
   bodn2c_c("SUN", &target_id, &found);
   bodn2c_c("EARTH", &observer_id, &found);

   spkez_c(target_id, input_double, "IAU_SUN", "LT", observer_id, pos_vel, &lt);

   return true;
}

bool CPPSpice::AreDateBoundsValid(
   const std::string& lower_bound, const std::string& upper_bound) {

   // If we've already gotten here, we can be confident that the string format is good
   // for literal comparison, so we can compare on the string
   if (lower_bound == upper_bound) {
      std::cout << "Error: lower and upper bounds are identical." << std::endl;
      return false;
   }

   if (lower_bound > upper_bound) {
      std::cout << "Error: lower bound is greater than the upper bound." << std::endl;
      return false;
   }

   return true;
}