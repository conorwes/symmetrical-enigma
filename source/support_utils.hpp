#pragma once

#include <regex>
#include <string>
#include <tuple>

#include "includes_common.hpp"

namespace CPPSpice {
   int GetNAIFIDfromName(const std::string& name);

   bool IsValidDate(const std::string& input);

   bool
   AreDateBoundsValid(const std::string& lower_bound, const std::string& upper_bound);

   bool FurnishSPICEKernel(const std::string& kernel_name);

   bool QueryParticipantDetails(
      const std::string&                                 participant_type,
      std::tuple<std::string, std::string, std::string>& participant_info);

   bool QueryConfigurationDetails(SimulationData& data);

   bool ParseConfigurationFile(const std::string& filename, SimulationData& data);

   // compiling regex is costly, so let's do it once
   const auto date_format =
      std::regex("[0-9]{4} [A-Z]{3} [0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}(?: TDB)?");

   // also define valid months and their corresponding number of days
   const std::vector<std::pair<std::string, int>> valid_months = {
      std::pair<std::string, int>("JAN", 31),
      std::pair<std::string, int>("FEB", 28),
      std::pair<std::string, int>("MAR", 31),
      std::pair<std::string, int>("APR", 30),
      std::pair<std::string, int>("MAY", 31),
      std::pair<std::string, int>("JUN", 30),
      std::pair<std::string, int>("JUL", 31),
      std::pair<std::string, int>("AUG", 31),
      std::pair<std::string, int>("SEP", 30),
      std::pair<std::string, int>("OCT", 31),
      std::pair<std::string, int>("NOV", 30),
      std::pair<std::string, int>("DEC", 31)};

   enum class DefinitionMode : int { Console, File };

   const std::vector<std::string> valid_occultation_types = {
      "FULL", "ANNULAR", "PARTIAL", "ANY"};
}   // namespace CPPSpice