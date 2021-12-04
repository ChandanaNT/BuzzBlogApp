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

#include <buzzblog/gen/TLikeService.h>


using apache::thrift;
using apache::thrift::protocol;
using apache::thrift::transport;

using gen;


namespace like_service {
class Client {
 private:
    std::string _ip_address;
    int _port;
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TLikeServiceClient> _client;

 public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _ip_address = ip_address;
      _port = port;
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TLikeServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TLike like_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      TLike _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->like_post(_return, request_metadata, post_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=like:like_post latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    TLike retrieve_standard_like(const TRequestMetadata& request_metadata,
        const int32_t like_id) {
      TLike _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_standard_like(_return, request_metadata, like_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=like:retrieve_standard_like latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    TLike retrieve_expanded_like(const TRequestMetadata& request_metadata,
        const int32_t like_id) {
      TLike _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_expanded_like(_return, request_metadata, like_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=like:retrieve_expanded_like latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    void delete_like(const TRequestMetadata& request_metadata,
        const int32_t like_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->delete_like(request_metadata, like_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=like:delete_like latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
    }

    std::vector<TLike> list_likes(const TRequestMetadata& request_metadata,
        const TLikeQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TLike> _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->list_likes(_return, request_metadata, query, limit, offset);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=like:list_likes latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    int32_t count_likes_by_account(const TRequestMetadata& request_metadata,
        const int32_t account_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      auto ret = _client->count_likes_by_account(request_metadata, account_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=like:count_likes_by_account latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return ret;
    }

    int32_t count_likes_of_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      auto ret = _client->count_likes_of_post(request_metadata, post_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=like:count_likes_of_post latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return ret;
    }
};
}
