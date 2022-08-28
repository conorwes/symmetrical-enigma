// clang-format off
/*

- Header_File IncludesCommon.hpp (Miscellaneous support code)

- Abstract

   Define miscellaneous support code used elsewhere in the program, including
   any enums, structs, and other commonly-used bits of code.

- Disclaimer

   This code was created by me and is provided as-is.

- Required_Reading

   None.

- Particulars

   This file is an umbrella header that includes the header file required to
   interface with the CSPICE API, common standard library header files, and the
   definitions for several commonly-used members.

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

#pragma once

/*
Include these headers to support native c++ functionality.
*/
#include <iostream>
#include <regex>
#include <string>
#include <tuple>

#include <direct.h>

/*
Include SpiceUsr.h so we can interface with the CSPICE API.
*/
extern "C" {
#include "SpiceUsr.h"
}

/*
All of our non-program functionality lives within the cppspice namespace. This
will ensure that everything is made available appropriately.
*/
namespace cppspice {

   /*
   It's inconvenient to have to write out the tuple definition, so
   for convenience and clarity, we'll define ParticipantDetails here as well.
   */
   using ParticipantDetails =
      std::tuple<std::string, std::string, std::string>;

   /*
   Since this program supports console input and file parsing, it's useful
   to create a SimulationData struct to manage the required inputs for the
   occultation-search algorithm.
   */
   struct SimulationData {
      std::string        LowerBoundEpoch;
      std::string        UpperBoundEpoch;
      double             StepSize;
      std::string        OccultationType;
      ParticipantDetails OcculterDetails;
      ParticipantDetails TargetDetails;
      std::string        ObserverName;
      double             Tolerance;
   };

   /*
   This regex is useful for validating dates, and compiling regex is
   expensive. So, we'll define it once here.
   */
   const auto dateFormatRegex = std::regex(
      "[0-9]{4} [A-Z]{3} [0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}(?: TDB)?" );

   /*
   Typing out the std::pair definition is tedious and unclear. So, define
   MonthDetails here.
   */
   using MonthDetails = std::pair<std::string, int>;

   /*
   In various places we need to ensure that specified days and months are
   valid, and this is the most convenient way to do so.
   */
   const std::vector<std::pair<std::string, int>> validMonths = {
      MonthDetails( "JAN", 31 ),
      MonthDetails( "FEB", 28 ),
      MonthDetails( "MAR", 31 ),
      MonthDetails( "APR", 30 ),
      MonthDetails( "MAY", 31 ),
      MonthDetails( "JUN", 30 ),
      MonthDetails( "JUL", 31 ),
      MonthDetails( "AUG", 31 ),
      MonthDetails( "SEP", 30 ),
      MonthDetails( "OCT", 31 ),
      MonthDetails( "NOV", 30 ),
      MonthDetails( "DEC", 31 ) };

   /*
   We have different functionality depending on the definition mode, and the
   most robust way to check for validity is to use an enum.
   */
   enum class DefinitionMode : int {
      CONSOLE,
      FILE
   };

   /*
   Occultation type is useful in several places, so it is useful to have a
   defined list of valid options here.
   */
   const std::vector<std::string> validOccultationTypes =
      { "FULL", "ANNULAR", "PARTIAL", "ANY" };

   /*
   Shape type is used in the occultation analysis.
   */
   // TODO - add support for DSK/UNPRIORITIZED
   const std::vector<std::string> validShapeTypes = {
      "ELLIPSOID",
      "POINT" /*, "DSK/UNPRIORITIZED"*/ };
}   // namespace cppspice
    /* End IncludesCommon.hpp */