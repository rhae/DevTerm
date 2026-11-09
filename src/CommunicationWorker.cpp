#include "CommunicationWorker.h"

#include <chrono>
#include <memory>

CCommunicationWorker::CCommunicationWorker()
  : m_Running(false)
{
}

CCommunicationWorker::~CCommunicationWorker()
{
  Stop();
}

void CCommunicationWorker::Start()
{
  if (m_Running)
    return;
  m_Running = true;
  m_Thread = std::thread(&CCommunicationWorker::Run, this);
}

void CCommunicationWorker::Stop()
{
  if (!m_Running)
    return;
  Enqueue(CommandStop, "", 0);
  if (m_Thread.joinable())
    m_Thread.join();
  m_Running = false;
}

void CCommunicationWorker::ConnectTcp(const std::string& Host, int Port)
{
  Enqueue(CommandConnectTcp, Host, Port);
}

void CCommunicationWorker::ConnectSerial(const std::string& Device, int BaudRate)
{
  Enqueue(CommandConnectSerial, Device, BaudRate);
}

void CCommunicationWorker::Disconnect()
{
  Enqueue(CommandDisconnect, "", 0);
}

void CCommunicationWorker::Send(const std::string& Command)
{
  Enqueue(CommandSend, Command, 0);
}

void CCommunicationWorker::TakeEvents(std::vector<CCommunicationEvent>& Events)
{
  std::lock_guard<std::mutex> Lock(m_Mutex);
  while (!m_Events.empty())
  {
    Events.push_back(m_Events.front());
    m_Events.pop_front();
  }
}

void CCommunicationWorker::Enqueue(ECommandType Type, const std::string& Text, int Value)
{
  std::lock_guard<std::mutex> Lock(m_Mutex);
  CCommand Command;
  Command.Type = Type;
  Command.Text = Text;
  Command.Value = Value;
  m_Commands.push_back(Command);
}

void CCommunicationWorker::AddEvent(ECommunicationEventType Type, const std::string& Text)
{
  std::lock_guard<std::mutex> Lock(m_Mutex);
  m_Events.push_back(CCommunicationEvent(Type, Text));
}

void CCommunicationWorker::Run()
{
  std::unique_ptr<CConnection> Connection;
  bool Connected = false;
  bool KeepRunning = true;

  while (KeepRunning)
  {
    CCommand Command;
    bool HasCommand = false;
    {
      std::lock_guard<std::mutex> Lock(m_Mutex);
      if (!m_Commands.empty())
      {
        Command = m_Commands.front();
        m_Commands.pop_front();
        HasCommand = true;
      }
    }

    if (!HasCommand)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      if (!Connected)
        continue;

      std::string Response;
      std::string Error;
      if (Connection->ReadLine(Response, 100, Error))
        AddEvent(CommunicationEventResponse, Response);
      else if (!Error.empty())
      {
        AddEvent(CommunicationEventError, Error);
        Connection->Close();
        Connection.reset();
        Connected = false;
        AddEvent(CommunicationEventDisconnected, "");
      }
      continue;
    }

    if (Command.Type == CommandStop)
    {
      KeepRunning = false;
      continue;
    }

    if (Command.Type == CommandDisconnect)
    {
      if (Connection.get() != NULL)
        Connection->Close();
      Connection.reset();
      Connected = false;
      AddEvent(CommunicationEventDisconnected, "");
      continue;
    }

    if (Command.Type == CommandConnectTcp || Command.Type == CommandConnectSerial)
    {
      if (Connection.get() != NULL)
        Connection->Close();
      Connected = false;
      if (Command.Type == CommandConnectTcp)
        Connection.reset(new CTcpConnection());
      else
        Connection.reset(new CSerialConnection());

      std::string Error;
      if (Connection->Open(Command.Text, Command.Value, Error))
      {
        Connected = true;
        AddEvent(CommunicationEventConnected, "");
        AddEvent(CommunicationEventLog, "Verbunden.");
      }
      else
      {
        AddEvent(CommunicationEventError, Error);
        Connection.reset();
        AddEvent(CommunicationEventDisconnected, "");
      }
      continue;
    }

    if (Command.Type == CommandSend)
    {
      if (!Connected || Connection.get() == NULL)
      {
        AddEvent(CommunicationEventError, "Keine Verbindung zum Senden.");
        continue;
      }

      AddEvent(CommunicationEventLog, "> " + Command.Text);
      std::string Error;
      if (!Connection->Write(Command.Text + "\n", Error))
      {
        AddEvent(CommunicationEventError, Error);
        Connection->Close();
        Connection.reset();
        Connected = false;
        AddEvent(CommunicationEventDisconnected, "");
      }
    }
  }

  if (Connection.get() != NULL)
    Connection->Close();
}
