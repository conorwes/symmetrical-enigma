#pragma once

#include "IncludesCommon.hpp"

namespace cppspice {
   int getNAIFIDFromName( const std::string& name );

   bool isValidDate( const std::string& input );

   bool areValidDateBounds(
      const std::string& lowerBound,
      const std::string& upperBound );

   bool furnishSPICEKernel( const std::string& kernelName );

   bool queryParticipantDetails(
      const std::string&  participantType,
      ParticipantDetails& participantInfo );

   bool queryConfigurationDetails( SimulationData& data );

   bool parseConfigurationFile(
      const std::string& filename,
      SimulationData&    data );

   void disambiguateRelativePath( std::string& path );
}   // namespace cppspice