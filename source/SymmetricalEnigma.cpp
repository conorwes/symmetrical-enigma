// clang-format off
/*

- Source_File SymmetricalEnigma.cpp (Main program code)

- Abstract

   Implement the main function which drives this program.

- Disclaimer

   This code was created by me and is provided as-is.

- Required_Reading

   FRAMES
   GF
   KERNEL
   NAIF_IDS
   SPK
   TIME
   WINDOWS

- Particulars

   This file contains the implementation of the main function for this program.
   The main function contains some basic user input that is required to
   direct the workflow into one of two directions - either user-input into the
   console, or user-input via a configuration file.

- Literature_References

   None.

- Author

   C.P. Westphal     (self)

- Credits

   This file references the CSPICE API, which was developed by the NAIF at
   JPL.

- Restrictions

   None.

- Version

   -Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)

*/
// clang-format on

/*
Include the two support headers.
*/
#include "OccultationUtils.hpp"
#include "SupportUtils.hpp"

/*
Additionally, we want to use the cppspice namespace.
*/
using namespace cppspice;

/*
This is the main function of this program.
*/
int main() {
   /*
   - Detailed_Input

      none.

   - Detailed_Output

      Returns a status code in the form of an int.

   - Error Handling

      If a CSPICE routine encounters an error, the native CSPICE error handler
   will be responsible for error handling. Otherwise, we report an error and
   return an error code.

   - Particulars

      None.

   - Literature_References

      None.

   - Author

      C.P. Westphal     (self)

   - Credits

      This file references the CSPICE API, which was developed by the NAIF at
      JPL.

   - Restrictions

      None.

   - Version

      Symmetrical-Enigma Version 1.0.0, 28-AUG-2022 (CPW)
   */

   /*
   We'll query a few things here, so start off by initializing an input
   receiver string.
   */
   std::string input{ "" };

   /*
   Firstly, let's determine whether we're going to input our data via the
   console or a file.
   */
   std::cout << "How would you like to specify your parameters?" << std::endl;
   std::cout << "- Console (c)" << std::endl;
   std::cout << "- File (f)" << std::endl;
   std::getline( std::cin, input );

   /*
   For the sake of comparison, let's convert to uppercase.
   */
   std::transform(
      input.begin(),
      input.end(),
      input.begin(),
      []( unsigned char c ) {
         return std::toupper( c );
      } );

   /*
   Compare our input against the valid input types.
   */
   DefinitionMode definitionMode{ DefinitionMode::CONSOLE };
   if ( input == "CONSOLE" || input == "C" ) {
      definitionMode = DefinitionMode::CONSOLE;
   }
   else if ( input == "FILE" || input == "F" ) {
      definitionMode = DefinitionMode::FILE;
   }
   else {
      std::cout << "Error: the specified definition mode '" << input
                << "' is invalid." << std::endl;
      return 1;
   }

   /*
   Next, let's choose whether we're using the CSPICE routine or the custom
   algorithm.
   */
   std::cout << "Which algorithm would you like to use?" << std::endl;
   std::cout << "- Custom (c)" << std::endl;
   std::cout << "- SPICE (s)" << std::endl;
   std::getline( std::cin, input );

   /*
   For the sake of comparison, let's convert to uppercase.
   */
   std::transform( input.begin(), input.end(), input.begin(), ::toupper );
   AlgorithmChoice algorithmChoice{ AlgorithmChoice::CUSTOM };
   if ( input == "CUSTOM" || input == "C" ) {
      algorithmChoice = AlgorithmChoice::CUSTOM;
   }
   else if (
      input == "SPICE" || input == "S" ||
      input == "CSPICE" /*we'll be generous here*/ )
   {
      algorithmChoice = AlgorithmChoice::SPICE;
   }
   else {
      std::cout << "Error: the specified algorithm choice '" << input
                << "' is invalid." << std::endl;
      return 1;
   }

   /*
   Finally, we hit the fork in the road, so let's operate on that logic.
   Initialize objects which are used across both forks here.
   */
   SimulationData data;
   if ( definitionMode == DefinitionMode::CONSOLE ) {
      /*
      If we're working with console inputs, initialize the data here, and then
      drop into the queryConfigDetails function.
      */
      if ( !queryConfigDetails( data, algorithmChoice ) ) {
         return 1;
      }
   }
   else {
      /*
      If we're working with a configuration file, we need to first find the
      file in question.
      */
      std::cout << "Specify your configuration file's path: " << std::endl;
      std::getline( std::cin, input );

      /*
      In case we have a relative directory, let's disambiguate just in case.
      */
      disambigRelPath( input );

      /*
      Now, check if the file exists.
      */
      if ( FILE* file = fopen( input.c_str(), "r" ) ) {
         fclose( file );
      }
      else {
         std::cout << "Error: the specified specification file '" << input
                   << "' could not be located." << std::endl;
         return 1;
      }

      /*
      At this point, we're confident the file exists, so let's drop into
      parseConfigFile to configure our SimulationData.
      */
      if ( !cppspice::parseConfigFile( input, data ) )
         return 1;
   }

   /*
   Finally, the moment we've all been waiting for: let's perform our search.
   */
   if ( algorithmChoice == AlgorithmChoice::CUSTOM ) {
      cppspice::performCustOccSrch( data );
   }
   else {
      SpiceCell* results;
      results = cppspice::performCSPICEOccSrch( data );

      /*
      Now that we have our results, we can go ahead and report the data.
      */
      cppspice::reportSearchSummary( results );
   }

   return 0;
}
/* End SymmetricalEnigma.cpp */