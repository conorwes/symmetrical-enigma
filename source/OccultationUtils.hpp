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
   This is the function which is used to perform the occultation search. We
   feed in the SimulationData which was retrieved prior to this call.
   */
   SpiceCell* performOccultationSearch( const SimulationData& data );

   /*
   This is a function which can be used to iterate through the results from
   the occultation search and report the relevant statistics.
   */
   void reportSearchSummary( SpiceCell* result );
}   // namespace cppspice
    /* End OccultationUtils.hpp */