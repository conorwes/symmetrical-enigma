#include <algorithm>
#include <fstream>

#include "support_utils.hpp"

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

bool CPPSpice::FurnishSPICEKernel(const std::string& kernel_name) {
   std::string path;
   std::cout << "Specify the " << kernel_name << " kernel's path: " << std::endl;
   std::getline(std::cin, path);

   // Before feeding this kernel into CSPICE, let's make sure it actually exists
   DisambiguateRelativePath(path);
   if (FILE* file = fopen(path.c_str(), "r")) {
      fclose(file);
   }
   else {
      std::cout << "Error: the specified kernel '" << path << "' could not be located."
                << std::endl;
      return false;
   }

   // Call the CSPICE furnsh_c routine. No need to error check, since the CSPICE
   // routines are self-reporting.
   furnsh_c(path.c_str());
   return true;
}

bool CPPSpice::QueryParticipantDetails(
   const std::string&                                 participant_type,
   std::tuple<std::string, std::string, std::string>& participant_info) {
   std::string input;
   std::cout << participant_type << " Body: " << std::endl;
   std::getline(std::cin, input);
   if (GetNAIFIDfromName(input) == -1) {
      std::cout << "Error: the specified body name '" << input
                << "' does not correspond to a valid NAIF object." << std::endl;
      return false;
   }
   std::string participant_name = input;

   std::cout << participant_type << " Body Shape: " << std::endl;

   // TODO - add support for DSK/UNPRIORITIZED
   std::vector<std::string> valid_shapes = {
      "ELLIPSOID", "POINT" /*, "DSK/UNPRIORITIZED"*/};

   for (auto& s : valid_shapes) {
      std::cout << "- " << s << std::endl;
   }
   std::getline(std::cin, input);

   auto shape_it = std::find_if(
      valid_shapes.begin(), valid_shapes.end(), [&input](const std::string& shape) {
         return shape == input;
      });

   if (shape_it == valid_shapes.end()) {
      std::cout << "Error: the specified body shape '" << input
                << "' is not a valid option." << std::endl;
      return false;
   };
   std::string participant_body_shape = input;

   std::cout << participant_type << " Body Frame: " << std::endl;
   std::getline(std::cin, input);
   int frame_code{0};
   namfrm_c(input.c_str(), &frame_code);
   if (frame_code == 0) {
      std::cout << "Error: the specified body frame: '" << input << "' is not recognized."
                << std::endl;
      return false;
   }
   std::string participant_body_frame = input;

   participant_info =
      std::make_tuple(participant_name, participant_body_shape, participant_body_frame);

   return true;
}

bool CPPSpice::QueryConfigurationDetails(SimulationData& data) {

   std::string input;

   // Before we do anything else, let's furnish the kernels
   // we'll need for this
   // program. For this program, we'll need a pck, tsp, and bsp file, so furnish those
   // now
   if (!FurnishSPICEKernel("P Constants"))
      return false;
   if (!FurnishSPICEKernel("Timespan"))
      return false;
   if (!FurnishSPICEKernel("Planetary Ephemerides"))
      return false;

   // Now we're ready to roll. Let's get the other things we'll need here.

   // Let's get the date range and perform some validation
   std::cout << "Lower Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::getline(std::cin, input);
   if (!IsValidDate(input)) {
      return false;
   }
   data.LowerBoundEpoch = input;

   std::cout << "Upper Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::getline(std::cin, input);
   if (!IsValidDate(input)) {
      return false;
   }
   data.UpperBoundEpoch = input;

   if (!AreDateBoundsValid(data.LowerBoundEpoch, data.UpperBoundEpoch)) {
      return false;
   }

   // Now we'll get the step size
   std::cout << "Step Size (s): " << std::endl;
   std::getline(std::cin, input);
   data.StepSize = std::atof(input.c_str());
   if (data.StepSize < 0.0) {
      std::cout << "Error: negative step sizes are not supported." << std::endl;
      return false;
   }

   // Next retrieve the occultation type and validate
   std::cout << "Occultation Type: " << std::endl;

   for (auto& type : valid_occultation_types) {
      std::cout << "- " << type << std::endl;
   }
   std::getline(std::cin, input);

   auto type_it = std::find_if(
      valid_occultation_types.begin(),
      valid_occultation_types.end(),
      [&input](const std::string& type) {
         return type == input;
      });

   if (type_it == valid_occultation_types.end()) {
      std::cout << "Error: the specified occultation type '" << input
                << "' is not a valid option." << std::endl;
      return false;
   };

   data.OccultationType = input;

   if (!QueryParticipantDetails("Occulting", data.OcculterDetails))
      return false;

   if (!QueryParticipantDetails("Target", data.TargetDetails))
      return false;

   // get the observer
   std::cout << "Observing Body: " << std::endl;
   std::getline(std::cin, input);
   if (GetNAIFIDfromName(input) == -1) {
      std::cout << "Error: the specified body name '" << input
                << "' does not correspond to a valid NAIF object." << std::endl;
      return false;
   }
   data.ObserverName = input;

   // get the tolerance
   std::cout << "Tolerance: " << std::endl;
   std::getline(std::cin, input);
   data.Tolerance = std::atof(input.c_str());
   if (data.Tolerance < 0.0) {
      std::cout << "Error: the tolerance value '" << data.Tolerance
                << "' is less than zero." << std::endl;
      return false;
   }

   return true;
}

bool CPPSpice::ParseConfigurationFile(const std::string& filename, SimulationData& data) {
   std::ifstream            file(filename);
   std::string              line;
   std::vector<std::string> file_contents;
   while (std::getline(file, line)) {
      file_contents.push_back(line);
   }

   char delimiter = ':';
   auto trim_left = [](std::string& s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                 return !std::isspace(ch);
              }));
   };

   auto trim_right = [](std::string& s) {
      s.erase(
         std::find_if(
            s.rbegin(),
            s.rend(),
            [](unsigned char ch) {
               return !std::isspace(ch);
            })
            .base(),
         s.end());
   };

   auto trim = [trim_left, trim_right](std::string& s) {
      trim_left(s);
      trim_right(s);
   };

   std::string identifier, content;

   for (auto& c : file_contents) {
      identifier = c.substr(0, c.find(delimiter));
      content    = c.substr(c.find(delimiter) + 1, c.length());
      trim(identifier);
      trim(content);
      if (identifier == "PConstants") {
         DisambiguateRelativePath(content);
         furnsh_c(content.c_str());
      }
      else if (identifier == "Timespan") {
         DisambiguateRelativePath(content);
         furnsh_c(content.c_str());
      }
      else if (identifier == "PlanetaryEphemerides") {
         DisambiguateRelativePath(content);
         furnsh_c(content.c_str());
      }
      else if (identifier == "LowerBoundEpoch") {
         if (!IsValidDate(content)) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
         data.LowerBoundEpoch = content;
      }
      else if (identifier == "UpperBoundEpoch") {
         if (!IsValidDate(content)) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
         data.UpperBoundEpoch = content;
      }
      else if (identifier == "StepSize") {
         data.StepSize = std::atof(content.c_str());
         if (data.StepSize < 0.0) {
            std::cout << "Error: The value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
      }
      else if (identifier == "OccultationType") {
         auto type_it = std::find_if(
            valid_occultation_types.begin(),
            valid_occultation_types.end(),
            [&content](const std::string& type) {
               return type == content;
            });

         if (type_it == valid_occultation_types.end()) {
            std::cout << "Error: the value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         };
         data.OccultationType = content;
      }
      else if (identifier == "OccultingBodyShape") {
         // TODO - add some validation here
         std::get<1>(data.OcculterDetails) = content;
      }
      else if (identifier == "OccultingBodyFrame") {
         // TODO - add some validation here
         std::get<2>(data.OcculterDetails) = content;
      }
      else if (identifier == "OccultingBody") {
         if (GetNAIFIDfromName(content) == -1) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object." << std::endl;
            return false;
         }
         std::get<0>(data.OcculterDetails) = content;
      }
      else if (identifier == "TargetBodyShape") {
         // TODO - add some validation here
         std::get<1>(data.TargetDetails) = content;
      }
      else if (identifier == "TargetBodyFrame") {
         // TODO - add some validation here
         std::get<2>(data.TargetDetails) = content;
      }
      else if (identifier == "TargetBody") {
         if (GetNAIFIDfromName(content) == -1) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object." << std::endl;
            return false;
         }
         std::get<0>(data.TargetDetails) = content;
      }
      else if (identifier == "ObservingBody") {
         if (GetNAIFIDfromName(content) == -1) {
            std::cout << "Error: the specified body name '" << content
                      << "' does not correspond to a valid NAIF object." << std::endl;
            return false;
         }
         data.ObserverName = content;
      }
      else if (identifier == "Tolerance") {
         data.Tolerance = std::atof(content.c_str());
         if (data.Tolerance <= 0.0) {
            std::cout << "Error: the value specified for '" << identifier
                      << "' is invalid." << std::endl;
            return false;
         }
      }
      else {
         continue;
      }
   }

   return true;
}

void CPPSpice::DisambiguateRelativePath(std::string& path) {
   if (path[0] = '.') {
      std::string current_path  = _getcwd(NULL, 0);
      auto        source_offset = current_path.rfind("source");
      if (source_offset != std::string::npos) {
         current_path = current_path.substr(0, source_offset);
         path         = current_path + path.substr(2, path.length());
         std::replace(path.begin(), path.end(), '/', '\\');
      }
   }
}