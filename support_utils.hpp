#pragma once

#include <string>

namespace CPPSpice {
   int GetNAIFIDfromName(const std::string& name);

   bool IsValidDateFormat(const std::string& input);

   bool
   AreDateBoundsValid(const std::string& lower_bound, const std::string& upper_bound);
}   // namespace CPPSpice