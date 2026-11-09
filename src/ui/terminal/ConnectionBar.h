#pragma once

#include <string>

struct CConnectionSettings
{
  std::string Host;
  std::string Port;
  std::string SerialDevice;
  int BaudRate;
  int Transport;
};

enum CConnectionAction
{
  ConnectionActionNone,
  ConnectionActionConnect,
  ConnectionActionDisconnect,
  ConnectionActionClearLog
};

class CConnectionBar
{
public:
  CConnectionAction Render(CConnectionSettings& Settings,
                           bool Connected) const;
};
