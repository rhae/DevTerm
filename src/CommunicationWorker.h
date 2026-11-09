#pragma once

#include "Connection.h"

#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

enum ECommunicationEventType
{
  CommunicationEventConnected,
  CommunicationEventDisconnected,
  CommunicationEventLog,
  CommunicationEventError,
  CommunicationEventResponse
};

struct CCommunicationEvent
{
  ECommunicationEventType Type;
  std::string Text;

  CCommunicationEvent(ECommunicationEventType EventType, const std::string& EventText)
    : Type(EventType), Text(EventText)
  {
  }
};

class CCommunicationWorker
{
public:
  CCommunicationWorker();
  ~CCommunicationWorker();

  void Start();
  void Stop();
  void ConnectTcp(const std::string& Host, int Port);
  void ConnectSerial(const std::string& Device, int BaudRate);
  void Disconnect();
  void Send(const std::string& Command);
  void TakeEvents(std::vector<CCommunicationEvent>& Events);

private:
  enum ECommandType
  {
    CommandConnectTcp,
    CommandConnectSerial,
    CommandDisconnect,
    CommandSend,
    CommandStop
  };

  struct CCommand
  {
    ECommandType Type;
    std::string Text;
    int Value;
  };

  void Enqueue(ECommandType Type, const std::string& Text, int Value);
  void Run();
  void AddEvent(ECommunicationEventType Type, const std::string& Text);

  std::thread m_Thread;
  std::mutex m_Mutex;
  std::deque<CCommand> m_Commands;
  std::deque<CCommunicationEvent> m_Events;
  bool m_Running;
};
