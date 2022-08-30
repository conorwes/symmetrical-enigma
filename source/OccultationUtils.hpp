// clang-format off
/*

- Header_File OccultationUtils.hpp (Occultation utility code)

- Abstract

   Define utility functions which are used in the occultation analysis component
   of this program.

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

   This file is a header which defines the functions which are offered to 
   support occultation analysis.

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
Fortunately we only need to use the common includes for this file.
*/
#include "IncludesCommon.hpp"

/*
All of our non-program functionality lives within the cppspice namespace. This
will ensure that everything is made available appropriately.
*/
namespace cppspice {
   /*
   A function which determines whether the target is occulted at a specified
   epoch.
    */
   bool isOccultedAtEpoch(
      const SpiceInt    targetID,
      const SpiceInt    occulterID,
      const SpiceInt    observerID,
      const SpiceDouble epoch,
      const SpiceChar*  occulterFrame,
      const SpiceChar*  occulterName,
      const SpiceChar*  targetFrame,
      const SpiceChar*  targetName,
      SpiceBoolean&     isOcculted );

   /*
   A bisection algorithm to find the transition.
   */
   bool bisectEpochs(
      const SpiceInt     targetID,
      const SpiceInt     occulterID,
      const SpiceInt     observerID,
      const SpiceDouble  lowerEpoch,
      const SpiceBoolean lowerOcculted,
      const SpiceDouble  upperEpoch,
      const SpiceBoolean upperOcculted,
      const SpiceChar*   occulterFrame,
      const SpiceChar*   occulterName,
      const SpiceChar*   targetFrame,
      const SpiceChar*   targetName,
      const SpiceDouble  tolerance );

   /*
   This is a function which is used to perform the occultation search using
   a custom written algorithm.
    */
   bool performCustOccSrch( const SimulationData& data );

   /*
   This is the function which is used to perform the occultation search using
   the cspice gfoclt_c routine. We feed in the SimulationData which was
   retrieved prior to this call.
   */
   SpiceCell* performCSPICEOccSrch( const SimulationData& data );

   /*
   This is a function which can be used to iterate through the results from
   the occultation search and report the relevant statistics.
   */
   void reportSearchSummary( SpiceCell* result );
}   // namespace cppspice
    /* End OccultationUtils.hpp */