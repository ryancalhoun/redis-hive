#include "TcpServer.h"
#include "IEventBus.h"
#include "ITcpServerHandler.h"

namespace
{
  class Accept : public IEventBus::ICallback
  {
  public:
    Accept(TcpServer& server) : _server(server) {}
    void operator() () { _server.accept(); }

  protected:
    TcpServer& _server;
  };
}

TcpServer::TcpServer(ITcpServerHandler& handler, IEventBus& eventBus)
  : _handler(handler)
  , _eventBus(eventBus)
{}

void TcpServer::shutdown()
{
  _eventBus.remove(_server);
  _server.close();
}

bool TcpServer::listen(int port)
{
  if(! _server.listen(port)) {
    return false;
  }

  _eventBus.add(_server, new Accept(*this));
  return true;
}

void TcpServer::accept()
{
  TcpSocket client = _server.accept();

  if(client == -1) {
    return;
  }

  _handler.onAccept(client);
}

