/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2015. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#ifndef NT_DISPATCHER_H_
#define NT_DISPATCHER_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "llvm/StringRef.h"

#include "IDispatcher.h"
#include "INetworkConnection.h"

namespace wpi {
class Logger;
class NetworkAcceptor;
class NetworkStream;
}

namespace nt {

class IConnectionNotifier;
class IStorage;
class NetworkConnection;

class DispatcherBase : public IDispatcher {
  friend class DispatcherTest;

 public:
  typedef std::function<std::unique_ptr<wpi::NetworkStream>()> Connector;

  DispatcherBase(IStorage& storage, IConnectionNotifier& notifier,
                 wpi::Logger& logger);
  virtual ~DispatcherBase();

  unsigned int GetNetworkMode() const;
  void StartServer(llvm::StringRef persist_filename,
                   std::unique_ptr<wpi::NetworkAcceptor> acceptor);
  void StartClient();
  void Stop();
  void SetUpdateRate(double interval);
  void SetIdentity(llvm::StringRef name);
  void Flush();
  std::vector<ConnectionInfo> GetConnections() const;
  bool IsConnected() const;

  void SetConnector(Connector connector);
  void SetConnectorOverride(Connector connector);
  void ClearConnectorOverride();

  bool active() const { return m_active; }

  DispatcherBase(const DispatcherBase&) = delete;
  DispatcherBase& operator=(const DispatcherBase&) = delete;

 private:
  void DispatchThreadMain();
  void ServerThreadMain();
  void ClientThreadMain();

  bool ClientHandshake(
      NetworkConnection& conn,
      std::function<std::shared_ptr<Message>()> get_msg,
      std::function<void(llvm::ArrayRef<std::shared_ptr<Message>>)> send_msgs);
  bool ServerHandshake(
      NetworkConnection& conn,
      std::function<std::shared_ptr<Message>()> get_msg,
      std::function<void(llvm::ArrayRef<std::shared_ptr<Message>>)> send_msgs);

  void ClientReconnect(unsigned int proto_rev = 0x0300);

  void QueueOutgoing(std::shared_ptr<Message> msg, INetworkConnection* only,
                     INetworkConnection* except) override;

  IStorage& m_storage;
  IConnectionNotifier& m_notifier;
  unsigned int m_networkMode = NT_NET_MODE_NONE;
  std::string m_persist_filename;
  std::thread m_dispatch_thread;
  std::thread m_clientserver_thread;

  std::unique_ptr<wpi::NetworkAcceptor> m_server_acceptor;
  Connector m_client_connector_override;
  Connector m_client_connector;
  uint8_t m_connections_uid = 0;

  // Mutex for user-accessible items
  mutable std::mutex m_user_mutex;
  std::vector<std::shared_ptr<INetworkConnection>> m_connections;
  std::string m_identity;

  std::atomic_bool m_active;       // set to false to terminate threads
  std::atomic_uint m_update_rate;  // periodic dispatch update rate, in ms

  // Condition variable for forced dispatch wakeup (flush)
  std::mutex m_flush_mutex;
  std::condition_variable m_flush_cv;
  std::chrono::steady_clock::time_point m_last_flush;
  bool m_do_flush = false;

  // Condition variable for client reconnect (uses user mutex)
  std::condition_variable m_reconnect_cv;
  unsigned int m_reconnect_proto_rev = 0x0300;
  bool m_do_reconnect = true;

 protected:
  wpi::Logger& m_logger;
};

class Dispatcher : public DispatcherBase {
  friend class DispatcherTest;

 public:
  Dispatcher(IStorage& storage, IConnectionNotifier& notifier,
             wpi::Logger& logger)
      : DispatcherBase(storage, notifier, logger) {}

  void StartServer(StringRef persist_filename, const char* listen_address,
                   unsigned int port);

  void SetServer(const char* server_name, unsigned int port);
  void SetServer(ArrayRef<std::pair<StringRef, unsigned int>> servers);
  void SetServerTeam(unsigned int team, unsigned int port);

  void SetServerOverride(const char* server_name, unsigned int port);
  void ClearServerOverride();
};

}  // namespace nt

#endif  // NT_DISPATCHER_H_
