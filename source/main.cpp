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

   if (definition_mode == DefinitionMode::Console) {
      SimulationData data;
      if (!QueryConfigurationDetails(data)) {
         return 1;
      }

      results = CPPSpice::PerformOccultationSearch(std::move(data));
   }
   else {
      std::cout << "Specify your configuration file's path: " << std::endl;
      std::getline(std::cin, input);

      // when running from debugger, the working directory is source...which doesn't makes
      // sense, but VS code is giving me some trouble
      DisambiguateRelativePath(input);

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