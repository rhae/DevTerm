#include "ScpiResponse.h"

#include <cctype>
#include <cmath>
#include <cstdlib>

bool ParsePlotValue(const std::string& Text, double& Value)
{
  const char* Start = Text.c_str();
  if (Text.compare(0, 6, ":MEAS:") == 0)
  {
    size_t Space = Text.find_first_of(" \t", 6);
    if (Space == std::string::npos)
      return false;
    Start += Space;
  }

  char* End = NULL;
  double Parsed = std::strtod(Start, &End);
  while (End != NULL && std::isspace(static_cast<unsigned char>(*End)))
    ++End;
  if (End == Start || End == NULL || *End != '\0' || !std::isfinite(Parsed))
    return false;

  Value = Parsed;
  return true;
}
