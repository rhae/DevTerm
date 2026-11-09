#pragma once

#include "CCommunicationWorker.h"

#include <string>
#include <vector>

class CApplication
{
public:
  CApplication();
  ~CApplication();

  void Render();

private:
  void RenderMenu();
  void RenderToolbar();
  void RenderConnectionSettings();
  void RenderTabs();
  void RenderCommandInput();
  void RenderStatusBar();
  void ProcessEvents();
  void AddLogLine(const std::string& Line);
  void SendCommand();

  CCommunicationWorker m_Communication;
  std::string m_Host;
  std::string m_Port;
  std::string m_SerialDevice;
  int m_BaudRate;
  int m_Transport;
  bool m_Connected;
  bool m_AutoScroll;
  char m_Command[256];
  std::vector<std::string> m_Log;
  std::vector<double> m_PlotX;
  std::vector<double> m_PlotY;
  std::string m_Status;
};
