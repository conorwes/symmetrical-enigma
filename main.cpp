#include "support_utils.hpp"
#include "occultation_utils.hpp"

using namespace CPPSpice;

int main() {

   std::string input;
   SpiceCell*  results;

   std::cout << "How would you like to specify your parameters?" << std::endl;
   std::cout << "- Console" << std::endl;
   std::cout << "- File" << std::endl;
   std::getline(std::cin, input);
   std::transform(input.begin(), input.end(), input.begin(), ::toupper);
   DefinitionMode definition_mode;
   if (input == "CONSOLE") {
      definition_mode = DefinitionMode::Console;
   }
   else if (input == "FILE") {
      definition_mode = DefinitionMode::File;
   }
   else {
      std::cout << "Error: the specified definition mode '" << input << "' is invalid."
                << std::endl;
      return 1;
   }

   // TODO - let's make it so you can:
   // 1. specify the inputs directly into the console or
   // 2. read in a config file with everything you need

   if (definition_mode == DefinitionMode::Console) {
      SimulationData data;

      // Before we do anything else, let's furnish the kernels we'll need for this
      // program. For this program, we'll need a pck, tsp, and bsp file, so furnish those
      // now
      if (!FurnishSPICEKernel("P Constants"))
         return 1;
      if (!FurnishSPICEKernel("Timespan"))
         return 1;
      if (!FurnishSPICEKernel("Planetary Ephemerides"))
         return 1;

      // Now we're ready to roll. Let's get the other things we'll need here.

      // Let's get the date range and perform some validation
      std::cout << "Lower Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
      std::getline(std::cin, input);
      if (!IsValidDate(input)) {
         return 1;
      }
      data.LowerBoundEpoch = input;

      std::cout << "Upper Bound Epoch (YYYY MMM DD HH:MM:SS TDB): " << std::endl;
      std::getline(std::cin, input);
      if (!IsValidDate(input)) {
         return 1;
      }
      data.UpperBoundEpoch = input;

      if (!AreDateBoundsValid(data.LowerBoundEpoch, data.UpperBoundEpoch)) {
         return 1;
      }

      // Now we'll get the step size
      std::cout << "Step Size (s): " << std::endl;
      std::getline(std::cin, input);
      data.StepSize = std::atof(input.c_str());
      if (data.StepSize < 0.0) {
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

      data.OccultationType = input;

      if (!QueryParticipantDetails("Occulting", data.OcculterDetails))
         return 1;

      if (!QueryParticipantDetails("Target", data.TargetDetails))
         return 1;

      // get the observer
      std::cout << "Observing Body: " << std::endl;
      std::getline(std::cin, input);
      if (GetNAIFIDfromName(input) == -1) {
         std::cout << "Error: the specified body name '" << input
                   << "' does not correspond to a valid NAIF object." << std::endl;
         return 1;
      }
      data.ObserverName = input;

      // get the tolerance
      std::cout << "Tolerance: " << std::endl;
      std::getline(std::cin, input);
      data.Tolerance = std::atof(input.c_str());
      if (data.Tolerance < 0.0) {
         std::cout << "Error: the tolerance value '" << data.Tolerance
                   << "' is less than zero." << std::endl;
         return 1;
      }

      results = CPPSpice::PerformOccultationSearch(std::move(data));
   }
   else {
      std::cout << "Specify your configuration file's path: " << std::endl;
      std::getline(std::cin, input);
      if (FILE* file = fopen(input.c_str(), "r")) {
         fclose(file);
      }
      else {
         std::cout << "Error: the specified specification file '" << input
                   << "' could not be located." << std::endl;
         return 1;
      }

      SimulationData data;
      if (!CPPSpice::ParseConfigurationFile(input, data))
         return 1;

      results = CPPSpice::PerformOccultationSearch(std::move(data));
   }

   CPPSpice::ReportSummary(std::move(results));

   return 0;
}