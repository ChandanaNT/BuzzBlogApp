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

#include <buzzblog/gen/TPostService.h>

using apache::thrift;
using apache::thrift::protocol;
using apache::thrift::transport;

using gen;

namespace post_service {
class Client {
 private:
    std::string _ip_address;
    int _port;
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TPostServiceClient> _client;

 public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _ip_address = ip_address;
      _port = port;
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TPostServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TPost create_post(const TRequestMetadata& request_metadata,
        const std::string& text) {
      TPost _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->create_post(_return, request_metadata, text);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=post:create_post latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    TPost retrieve_standard_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      TPost _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_standard_post(_return, request_metadata, post_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=post:retrieve_standard_post latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    TPost retrieve_expanded_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      TPost _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->retrieve_expanded_post(_return, request_metadata, post_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=post:retrieve_expanded_post latency={}",
          request_metadata.id, _ip_address, _port, latency.count());
      return _return;
    }

    void delete_post(const TRequestMetadata& request_metadata,
        const int32_t post_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->delete_post(request_metadata, post_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=post:delete_post latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
    }

    std::vector<TPost> list_posts(const TRequestMetadata& request_metadata,
        const TPostQuery& query, const int32_t limit, const int32_t offset) {
      std::vector<TPost> _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->list_posts(_return, request_metadata, query, limit, offset);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=post:list_posts latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    int32_t count_posts_by_author(const TRequestMetadata& request_metadata,
        const int32_t author_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      auto ret = _client->count_posts_by_author(request_metadata, author_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=post:count_posts_by_author latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return ret;
    }
};
}
