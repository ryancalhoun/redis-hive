#include "Proxy.h"
#include "IEventBus.h"
#include "ILogger.h"
#include "IProxyHandler.h"

#include <cstdio> 
#include <cstdlib> 
#include <cstring> 

#include <iostream>

namespace
{
  class Read : public IEventBus::ICallback
  {
  public:
    Read(Proxy& proxy, void (Proxy::*cb)())
      : _proxy(proxy)
      , _cb(cb)
    {}

    void operator()() { (_proxy.*_cb)(); }

  protected:
    Proxy& _proxy;
    void (Proxy::*_cb)();
  };

  class Copy : public IEventBus::ICallback
  {
  public:
    Copy(Proxy& proxy, const TcpSocket& from, const TcpSocket& to)
      : _proxy(proxy)
      , _from(from)
      , _to(to)
    {}

    ~Copy() { _from.close(); }
    void operator()() { _proxy.copy(_from, _to); }

  protected:
    Proxy& _proxy;
    TcpSocket _from;
    TcpSocket _to;
  };

  std::string to_s(int port)
  {
    char val[8] = {0};
    ::snprintf(val, sizeof(val), "%i", port);
    return val;
  }

  std::string to_s(const std::string& address, int port)
  {
    return address + ":" + ::to_s(port);
  }
}

Proxy::Proxy(IEventBus& eventBus, int port, const std::string& auth, ILogger& logger)
  : _server(*this, eventBus)
  , _eventBus(eventBus)
  , _logger(logger)
  , _handler(NULL)
  , _local(port)
  , _auth(auth)
  , _interval(3000)
  , _commandInFlight(false)
  , _alone(true)
{
  proxyToLocal();
}

Proxy::~Proxy()
{
  shutdown();
}

void Proxy::shutdown()
{
  reset();
  _server.shutdown();
}

int Proxy::getLocalPort() const
{
  return _local;
}

void Proxy::proxyToLocal()
{
  _address = "127.0.0.1";
  _port = _local;

  runCommand("SLAVEOF NO ONE\r\n");;
  _alone = true;
}

void Proxy::proxyToAddress(const std::string& address, int port)
{
  if(address != _address || _port != port) {
    reset();

    _address = address;
    _port = port;
  }

  if(_alone) {
    runCommand("SLAVEOF " + _address + " " + ::to_s(_port) + "\r\n");
    _alone = false;
  }
}

void Proxy::reset()
{
  std::string previous = ::to_s(_address, _port);

  std::map<std::string, std::set<int> >::iterator it = _connections.find(previous);
  if(it != _connections.end()) {
    std::set<int>::const_iterator conn = it->second.begin();
    while(conn != it->second.end()) {
      _eventBus.remove(*conn);
      it->second.erase(conn++);
    }
    _connections.erase(it);
  }

  _alone = true;
}

bool Proxy::listen(int port)
{
  if(! _server.listen(port)) {
    return false;
  }

  _eventBus.every(_interval, new Read(*this, &Proxy::ping));

  std::cout << "Proxy listening on port " << port << std::endl;

  return true;
}

void Proxy::setHandler(IProxyHandler& handler)
{
  _handler = &handler;
}

void Proxy::onAccept(const TcpSocket& client)
{
  TcpSocket sock;
  if(! sock.connect(_address, _port)) {
    return;
  }

  _connections[::to_s(_address, _port)].insert(sock);

  _eventBus.add(client, new Copy(*this, client, sock));
  _eventBus.add(sock, new Copy(*this, sock, client));
}

void Proxy::copy(TcpSocket& from, TcpSocket& to)
{
  char buf[1024];
  if(from.read(buf, sizeof(buf))) {
    if(from.bytes() == 0) {
      _eventBus.remove(from);
      _eventBus.remove(to);
    } else {
      to.write(buf, from.bytes());
    }
  }
}

void Proxy::ping()
{
  runCommand("PING\r\n");
}

void Proxy::pong()
{
  char buff[1024] = {0};
  if(_redis.read(buff, sizeof(buff))) {
    _commandInFlight = false;
    if(_redis.bytes() == 0) {
      _logger.error("Redis disconnect");
      notReady();
    } else {
      ready();
      return runNextCommand();
    }
  }
}

void Proxy::runCommand(const std::string& command)
{
  if(_redis == -1) {
    if(! _redis.connect("127.0.0.1", _local)) {
      _logger.error("Redis connect error");
      return notReady();
    }
    _eventBus.add(_redis, new Read(*this, &Proxy::pong));

    if(_auth.size() > 0) {
      _commands.push_back("AUTH " + _auth + "\r\n");
    }
  }

  _commands.push_back(command);

  return runNextCommand();
}

void Proxy::runNextCommand()
{
  if(_commands.size() > 0 && ! _commandInFlight) {
    std::string command = _commands.front();
    if(_redis.write(command.c_str(), command.size())) {
      _commands.pop_front();
      _commandInFlight = true;
    } else {
      _logger.error("Redis send error");
      return notReady();
    }
  }
}

void Proxy::ready()
{
  if(_handler) {
    _handler->onProxyReady();
  }
}

void Proxy::notReady()
{
  _alone = true;
  if(_redis != -1) {
    _eventBus.remove(_redis);
    _redis.close();
  }

  if(_handler) {
    _handler->onProxyNotReady();
  }
}

