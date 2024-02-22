#include "Controller.h"
#include "IEventBus.h"
#include "ILogger.h"
#include "TcpSocket.h"

namespace
{
  class Ping : public IEventBus::ICallback
  {
  public:
    Ping(Controller& controller) : _controller(controller) {}
    void operator()() { _controller.ping(); }

  protected:
    Controller& _controller;
  };

  class Read : public IEventBus::ICallback
  {
  public:
    Read(Controller& controller, const TcpSocket& client)
      : _controller(controller)
      , _client(client)
    {}

    ~Read() { _client.close(); }

    void operator()() { _controller.read(_client); }

  protected:
    Controller& _controller;
    TcpSocket _client;
  };
}

Controller::Controller(IProxy& proxy, IEventBus& eventBus, const ICandidateList& candidates, const ITimeMachine& timeMachine, ILogger& logger)
  : _server(*this, eventBus)
  , _eventBus(eventBus)
  , _logger(logger)
  , _interval(5000)
  , _membership(*this, proxy, candidates, timeMachine, logger)
  , _currentClient(NULL)
{
}

void Controller::onAccept(const TcpSocket& client)
{
  _eventBus.add(client, new Read(*this, client));
}

bool Controller::listen(int port)
{
  if(! _server.listen(port)) {
    return false;
  }

  _eventBus.every(_interval, new Ping(*this));

  std::cout << "Controller listening on port " << port << std::endl;

  return true;
}

int Controller::read(TcpSocket& client)
{
  char buf[1024] = {0};
  if(! client.read(buf, sizeof(buf))) {
    return -1;
  }

  if(client.bytes() == 0) {
    _eventBus.remove(client);
  } else {
    _currentClient = &client;
    _membership.onRead(Packet(buf));
    _currentClient = NULL;
  }

  return 0;
}

int Controller::ping()
{
  _membership.onTimer();
  return 0;
}

bool Controller::ping(const std::string& addr, const Packet& packet)
{
  TcpSocket sock;

  std::string data = packet.serialize();

  if(! sock.connect(addr) || ! sock.write(data.c_str(), data.size())) {
    sock.close();
    return false;
  }

  _eventBus.add(sock, new Read(*this, sock));
  return true;
}

bool Controller::ack(const Packet& packet)
{
  if(! _currentClient) {
    return false;
  }

  std::string data = packet.serialize();

  if(! _currentClient->write(data.c_str(), data.size())) {
    return false;
  }

  _eventBus.remove(*_currentClient);
  return true;
}

