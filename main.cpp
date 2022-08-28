#include <tuple>

#include "support_utils.hpp"
#include "occultation_utils.hpp"

using namespace CPPSpice;

int main() {
   // TODO - let's make it so you can:
   // 1. specify the inputs directly into the console or
   // 2. read in a config file with everything you need

   // Define some helpful lambdas for this program
   // A simple lambda to prompt the user to specify a kernel path.
   auto furnish_spice_kernel = [](const std::string& kernel_name) -> bool {
      std::string path;
      std::cout << "Specify the " << kernel_name << " kernel's path: " << std::endl;
      std::getline(std::cin, path);

      // Before feeding this kernel into CSPICE, let's make sure it actually exists
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
   };

   // Before we do anything else, let's furnish the kernels we'll need for this program.
   // For this program, we'll need a pck, tsp, and bsp file, so furnish those now
   if (!furnish_spice_kernel("P Constants"))
      return 1;
   if (!furnish_spice_kernel("Timespan"))
      return 1;
   if (!furnish_spice_kernel("Planetary Ephemerides"))
      return 1;

   // Now we're ready to roll. Let's get the other things we'll need here.

   // Let's get the date range and perform some validation
   std::cout << "Lower Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::string input;
   std::getline(std::cin, input);
   if (!IsValidDate(input)) {
      return 1;
   }
   std::string startEpoch = input;

   std::cout << "Upper Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
   std::getline(std::cin, input);
   if (!IsValidDate(input)) {
      return 1;
   }
   std::string endEpoch = input;

   if (!AreDateBoundsValid(startEpoch, endEpoch)) {
      return 1;
   }

   // Now we'll get the step size
   std::cout << "Step Size (s): " << std::endl;
   std::getline(std::cin, input);
   double step_size = std::atof(input.c_str());
   if (step_size < 0.0) {
      std::cout << "Error: negative step sizes are not supported." << std::endl;
      return 1;
   }

   // Next retrieve the occultation type and validate
   std::cout << "Occultation Type: " << std::endl;
   std::vector<std::string> valid_types = {"FULL", "ANNULAR", "PARTIAL", "ANY"};

   for (auto& type : valid_types) {
      std::cout << "- " << type << std::endl;
   }
   std::getline(std::cin, input);

   auto type_it = std::find_if(
      valid_types.begin(), valid_types.end(), [&input](const std::string& type) {
         return type == input;
      });

   if (type_it == valid_types.end()) {
      std::cout << "Error: the specified occultation type '" << input
                << "' is not a valid option." << std::endl;
      return 1;
   };

   std::string occultation_type = input;

   auto retrieve_body_info =
      [](const std::string&                                 participant_type,
         std::tuple<std::string, std::string, std::string>& participant_info) -> bool {
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
      // TODO - add some validation
      std::getline(std::cin, input);
      int frame_code{0};
      namfrm_c(input.c_str(), &frame_code);
      if (frame_code == 0) {
         std::cout << "Error: the specified body frame: '" << input
                   << "' is not recognized." << std::endl;
         return false;
      }
      std::string participant_body_frame = input;

      participant_info = std::make_tuple(
         participant_name, participant_body_shape, participant_body_frame);

      return true;
   };

   std::tuple<std::string, std::string, std::string> occulting_information;
   if (!retrieve_body_info("Occulting", occulting_information))
      return 1;

   std::tuple<std::string, std::string, std::string> target_information;
   if (!retrieve_body_info("Target", target_information))
      return 1;

   // get the observer
   std::cout << "Observing Body: " << std::endl;
   std::getline(std::cin, input);
   if (GetNAIFIDfromName(input) == -1) {
      std::cout << "Error: the specified body name '" << input
                << "' does not correspond to a valid NAIF object." << std::endl;
      return 1;
   }
   std::string observing_name = input;

   // get the tolerance
   std::cout << "Tolerance: " << std::endl;
   std::getline(std::cin, input);
   double tolerance = std::atof(input.c_str());
   if (tolerance < 0.0) {
      std::cout << "Error: the tolerance value '" << tolerance << "' is less than zero."
                << std::endl;
      return 1;
   }

   auto results = CPPSpice::PerformOccultationSearch(
      startEpoch,
      endEpoch,
      step_size,
      occultation_type,
      std::get<0>(occulting_information),
      std::get<1>(occulting_information),
      std::get<2>(occulting_information),
      std::get<0>(target_information),
      std::get<1>(target_information),
      std::get<2>(target_information),
      "LT",
      observing_name,
      tolerance);

   CPPSpice::ReportSummary(std::move(results));

   return 0;
}