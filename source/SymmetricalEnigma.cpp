#include "SupportUtils.hpp"
#include "OccultationUtils.hpp"

using namespace cppspice;

int main() {

   std::string input;
   SpiceCell*  results;

   std::cout << "How would you like to specify your parameters?" << std::endl;
   std::cout << "- Console" << std::endl;
   std::cout << "- File" << std::endl;
   std::getline( std::cin, input );
   std::transform( input.begin(), input.end(), input.begin(), ::toupper );
   DefinitionMode definitionMode;
   if ( input == "CONSOLE" ) {
      definitionMode = DefinitionMode::CONSOLE;
   }
   else if ( input == "FILE" ) {
      definitionMode = DefinitionMode::FILE;
   }
   else {
      std::cout << "Error: the specified definition mode '" << input
                << "' is invalid." << std::endl;
      return 1;
   }

   if ( definitionMode == DefinitionMode::CONSOLE ) {
      SimulationData data;
      if ( !queryConfigurationDetails( data ) ) {
         return 1;
      }

      results = cppspice::performOccultationSearch( std::move( data ) );
   }
   else {
      std::cout << "Specify your configuration file's path: " << std::endl;
      std::getline( std::cin, input );

      // when running from debugger, the working directory is source...which
      // doesn't makes sense, but VS code is giving me some trouble
      disambiguateRelativePath( input );

      if ( FILE* file = fopen( input.c_str(), "r" ) ) {
         fclose( file );
      }
      else {
         std::cout << "Error: the specified specification file '" << input
                   << "' could not be located." << std::endl;
         return 1;
      }

      SimulationData data;
      if ( !cppspice::parseConfigurationFile( input, data ) )
         return 1;

      results = cppspice::performOccultationSearch( std::move( data ) );
   }

   cppspice::reportSearchSummary( std::move( results ) );

   return 0;
}