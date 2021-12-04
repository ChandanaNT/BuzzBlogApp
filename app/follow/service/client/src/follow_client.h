// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include <buzzblog/gen/TFollowService.h>


using apache::thrift;
using apache::thrift::protocol;
using apache::thrift::transport;

using gen;


namespace follow_service {
class Client {
 private:
    std::string _ip_address;
    int _port;
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TFollowServiceClient> _client;

 public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _ip_address = ip_address;
      _port = port;
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TFollowServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TFollow follow_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      TFollow _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->follow_account(_return, request_metadata, account_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:follow_account latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    TFollow retrieve_standard_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      TFollow _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_standard_follow(_return, request_metadata, follow_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:retrieve_standard_follow latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    TFollow retrieve_expanded_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      TFollow _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_expanded_follow(_return, request_metadata, follow_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:retrieve_expanded_follow latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    void delete_follow(const TRequestMetadata& request_metadata,
        const int32_t follow_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->delete_follow(request_metadata, follow_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:delete_follow latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
    }

    std::vector<TFollow> list_follows(const TRequestMetadata& request_metadata,
        const TFollowQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TFollow> _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->list_follows(_return, request_metadata, query, limit, offset);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:list_follows latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    bool check_follow(const TRequestMetadata& request_metadata,
        const int32_t follower_id, const int32_t followee_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      auto ret = _client->check_follow(request_metadata, follower_id,
          followee_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:check_follow latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return ret;
    }

    int32_t count_followers(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      auto ret = _client->count_followers(request_metadata, account_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:count_followers latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return ret;
    }

    int32_t count_followees(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      auto ret = _client->count_followees(request_metadata, account_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=follow:count_followees latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return ret;
    }
};
}
