// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <chrono>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <buzzblog/gen/TAccountService.h>


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace gen;


namespace account_service {
class Client {
 private:
    std::string _ip_address;
    int _port;
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TAccountServiceClient> _client;
 public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _ip_address = ip_address;
      _port = port;
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TAccountServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TAccount authenticate_user(const TRequestMetadata& request_metadata,
        const std::string& username, const std::string& password) {
      TAccount _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->authenticate_user(_return, request_metadata, username, password);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=account:authenticate_user latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    TAccount create_account(const TRequestMetadata& request_metadata,
        const std::string& username, const std::string& password,
        const std::string& first_name, const std::string& last_name) {
      TAccount _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->create_account(_return, request_metadata, username, password,
          first_name, last_name);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=account:create_account latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    TAccount retrieve_standard_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      TAccount _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_standard_account(_return, request_metadata, account_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=account:retrieve_standard_account latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    TAccount retrieve_expanded_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      TAccount _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_expanded_account(_return, request_metadata, account_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=account:retrieve_expanded_account latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    TAccount update_account(const TRequestMetadata& request_metadata,
        const int32_t account_id, const std::string& password,
        const std::string& first_name, const std::string& last_name) {
      TAccount _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->update_account(_return, request_metadata, account_id, password,
          first_name, last_name);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=account:update_account latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    void delete_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->delete_account(request_metadata, account_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=account:delete_account latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
    }
  };
}  // namespace account_service
