#pragma once

#include "CommunicationWorker.h"
#include "ui/shell/MenuBar.h"
#include "ui/shell/StatusBar.h"
#include "ui/shell/TitleBar.h"
#include "ui/terminal/CommandInput.h"
#include "ui/terminal/ConnectionBar.h"
#include "ui/terminal/WorkspaceTabs.h"

#include <string>
#include <vector>
#include <windows.h>

class CApplication
{
public:
  CApplication();
  ~CApplication();

  void Render(HWND Window, bool Active, bool TransparentBackground);

private:
  void HandleConnectionAction(CConnectionAction Action);
  void ProcessEvents();
  void AddLogLine(const std::string& Line);
  void SendCommand();

  CCommunicationWorker m_Communication;
  CConnectionSettings m_ConnectionSettings;
  bool m_Connected;
  bool m_AutoScroll;
  char m_Command[256];
  std::vector<std::string> m_Log;
  std::vector<double> m_PlotX;
  std::vector<double> m_PlotY;
  std::string m_Status;
  CTitleBar m_TitleBar;
  CMenuBar m_MenuBar;
  CConnectionBar m_ConnectionBar;
  CWorkspaceTabs m_WorkspaceTabs;
  CCommandInput m_CommandInput;
  CStatusBar m_StatusBar;
};
