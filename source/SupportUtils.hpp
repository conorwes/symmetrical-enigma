// clang-format off
/*

- Header_File SupportUtils.hpp (Support utility code)

- Abstract

   Define utility functions which are not specific to occultation analysis.

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

   This file is a header which defines utility functions which are not 
   necessarily involved in the occultation analyses performed by this 
   program. These functions are especially useful for validation for user
   inputs, gathering inputs, parsing files, and furnishing kernels.

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
   This is a utility to check for a NAIF ID based upon a provided body name.
   Ultimately this just gets used for validation that we're working with real
   objects.
   */
   int getNAIFIDFromName( const std::string& name );

   /*
   This function is used to validate a specified date to ensure that we're
   working with a valid epoch.
   */
   bool isValidDate( const std::string& input );

   /*
   This function is a second bit of date validation, namely to compare the
   specified upper and lower bounds to ensure that the dates involved are
   ready for comparison.
   */
   bool areValidDateBounds(
      const std::string& lowerBound,
      const std::string& upperBound );

   /*
   This is a helper function which queries users for a kernel, and then
   furnishes the kernel if it exists.
   */
   bool furnishSPICEKernel( const std::string& kernelName );

   /*
   This is a function to query a user for body details for one of the
   participants in the occultation analysis. Validation is also performed as
   part of this function.
   */
   bool queryParticipantDetails(
      const std::string&  participantType,
      ParticipantDetails& participantInfo,
      AlgorithmChoice&    choice );

   /*
   This is a utility to query a user for all of the user-specified components
   of the analysis using the console. The data are then used to populate the
   SimulationData which then gets fed into our occultation analysis.
   */
   bool
   queryConfigurationDetails( SimulationData& data, AlgorithmChoice& choice );

   /*
   This utility queries the user for a configuration file, which is then
   parsed. The data retrieved from the configuration file are fed into a
   SimulationData object, which is then used in the occulation analysis.
   */
   bool parseConfigurationFile(
      const std::string& filename,
      SimulationData&    data );

   /*
   This is a simple utility to take relative paths and ensure that they are
   translated to the correct path.
   */
   void disambiguateRelativePath( std::string& path );
}   // namespace cppspice
    /* End SupportUtils.hpp */