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

#include <buzzblog/gen/TUniquepairService.h>


using apache::thrift;
using apache::thrift::protocol;
using apache::thrift::transport;

using gen;


namespace uniquepair_service {
class Client {
 private:
    std::string _ip_address;
    int _port;
    std::shared_ptr<TSocket> _socket;
    std::shared_ptr<TTransport> _transport;
    std::shared_ptr<TProtocol> _protocol;
    std::shared_ptr<TUniquepairServiceClient> _client;

 public:
    Client(const std::string& ip_address, int port, int conn_timeout_ms) {
      _ip_address = ip_address;
      _port = port;
      _socket = std::make_shared<TSocket>(ip_address, port);
      _socket->setConnTimeout(conn_timeout_ms);
      _transport = std::make_shared<TBufferedTransport>(_socket);
      _protocol = std::make_shared<TBinaryProtocol>(_transport);
      _client = std::make_shared<TUniquepairServiceClient>(_protocol);
      _transport->open();
    }

    ~Client() {
      close();
    }

    void close() {
      if (_transport->isOpen())
        _transport->close();
    }

    TUniquepair get(const TRequestMetadata& request_metadata,
        const int32_t uniquepair_id) {
      TUniquepair _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->get(_return, request_metadata, uniquepair_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=uniquepair:get latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    TUniquepair add(const TRequestMetadata& request_metadata,
        const std::string& domain, const int32_t first_elem,
        const int32_t second_elem) {
      TUniquepair _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->add(_return, request_metadata, domain, first_elem, second_elem);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=uniquepair:add latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    void remove(const TRequestMetadata& request_metadata,
        const int32_t uniquepair_id) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->remove(request_metadata, uniquepair_id);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=uniquepair:remove latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
    }

    TUniquepair find(const TRequestMetadata& request_metadata,
        const std::string& domain, const int32_t first_elem,
        const int32_t second_elem) {
      TUniquepair _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->find(_return, request_metadata, domain, first_elem, second_elem);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=uniquepair:find latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    std::vector<TUniquepair> fetch(const TRequestMetadata& request_metadata,
        const TUniquepairQuery& query, const int32_t limit,
        const int32_t offset) {
      std::vector<TUniquepair> _return;
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      _client->fetch(_return, request_metadata, query, limit, offset);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=uniquepair:fetch latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return _return;
    }

    int32_t count(const TRequestMetadata& request_metadata,
        const TUniquepairQuery& query) {
      auto logger = spdlog::get("logger");
      auto start_time = std::chrono::steady_clock::now();
      auto ret = _client->count(request_metadata, query);
      std::chrono::duration<double> latency = \
          std::chrono::steady_clock::now() - start_time;
      logger->info("request_id={} server={}:{} "
          "function=uniquepair:count latency={}", request_metadata.id,
          _ip_address, _port, latency.count());
      return ret;
    }
};
}
