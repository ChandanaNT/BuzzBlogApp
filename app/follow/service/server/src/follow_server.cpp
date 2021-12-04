// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <cxxopts.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TFollowService.h>
#include <buzzblog/base_server.h>


using apache::thrift;
using apache::thrift::protocol;
using apache::thrift::transport;
using apache::thrift::server;

using gen;


class TFollowServiceHandler : public BaseServer, public TFollowServiceIf {
 public:
  TFollowServiceHandler(const std::string& backend_filepath,
      const std::string& postgres_user, const std::string& postgres_password,
      const std::string& postgres_dbname)
  : BaseServer(backend_filepath, postgres_user, postgres_password,
      postgres_dbname) {
  }

  void follow_account(TFollow& _return,
      const TRequestMetadata& request_metadata, const int32_t account_id) {
    // Add unique pair (follower, followee).
    auto uniquepair_client = get_uniquepair_client();
    TUniquepair uniquepair;
    try {
      uniquepair = uniquepair_client->add(request_metadata, "follow",
          request_metadata.requester_id, account_id);
    }
    catch (TUniquepairAlreadyExistsException e) {
      throw TFollowAlreadyExistsException();
    }
    uniquepair_client->close();

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = request_metadata.requester_id;
    _return.followee_id = account_id;
  }

  void retrieve_standard_follow(TFollow& _return,
      const TRequestMetadata& request_metadata, const int32_t follow_id) {
    // Get unique pair.
    auto uniquepair_client = get_uniquepair_client();
    TUniquepair uniquepair;
    try {
      uniquepair = uniquepair_client->get(request_metadata, follow_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }
    uniquepair_client->close();

    // Build follow (standard mode).
    _return.id = uniquepair.id;
    _return.created_at = uniquepair.created_at;
    _return.follower_id = uniquepair.first_elem;
    _return.followee_id = uniquepair.second_elem;
  }

  void retrieve_expanded_follow(TFollow& _return,
      const TRequestMetadata& request_metadata, const int32_t follow_id) {
    // Retrieve standard follow.
    retrieve_standard_follow(_return, request_metadata, follow_id);

    // Retrieve accounts.
    auto account_client = get_account_client();
    auto follower = account_client->retrieve_standard_account(request_metadata,
        _return.follower_id);
    auto followee = account_client->retrieve_standard_account(request_metadata,
        _return.followee_id);
    account_client->close();

    // Build follow (expanded mode).
    _return.__set_follower(follower);
    _return.__set_followee(followee);
  }

  void delete_follow(const TRequestMetadata& request_metadata,
      const int32_t follow_id) {
    {
      // Get unique pair.
      auto uniquepair_client = get_uniquepair_client();
      TUniquepair uniquepair;
      try {
        uniquepair = uniquepair_client->get(request_metadata, follow_id);
      }
      catch (TUniquepairNotFoundException e) {
        throw TFollowNotFoundException();
      }
      uniquepair_client->close();

      // Check if requester is authorized.
      if (request_metadata.requester_id != uniquepair.first_elem)
        throw TFollowNotAuthorizedException();
    }

    // Remove unique pair.
    auto uniquepair_client = get_uniquepair_client();
    try {
      uniquepair_client->remove(request_metadata, follow_id);
    }
    catch (TUniquepairNotFoundException e) {
      throw TFollowNotFoundException();
    }
    uniquepair_client->close();
  }

  void list_follows(std::vector<TFollow>& _return,
      const TRequestMetadata& request_metadata, const TFollowQuery& query,
      const int32_t limit, const int32_t offset) {
    // Build query struct.
    TUniquepairQuery uniquepair_query;
    uniquepair_query.__set_domain("follow");
    if (query.__isset.follower_id)
      uniquepair_query.__set_first_elem(query.follower_id);
    if (query.__isset.followee_id)
      uniquepair_query.__set_second_elem(query.followee_id);

    // Fetch unique pairs.
    auto uniquepair_client = get_uniquepair_client();
    std::vector<TUniquepair> uniquepairs = uniquepair_client->fetch(
        request_metadata, uniquepair_query, limit, offset);
    uniquepair_client->close();

    // Build follows.
    auto account_client = get_account_client();
    for (auto it : uniquepairs) {
      // Retrieve accounts.
      auto follower = account_client->retrieve_standard_account(
          request_metadata, it.first_elem);
      auto followee = account_client->retrieve_standard_account(
          request_metadata, it.second_elem);

      // Build follow (expanded mode).
      TFollow follow;
      follow.id = it.id;
      follow.created_at = it.created_at;
      follow.follower_id = it.first_elem;
      follow.followee_id = it.second_elem;
      follow.__set_follower(follower);
      follow.__set_followee(followee);
      _return.push_back(follow);
    }
    account_client->close();
  }

  bool check_follow(const TRequestMetadata& request_metadata,
      const int32_t follower_id, const int32_t followee_id) {
    bool follow_exists;
    auto uniquepair_client = get_uniquepair_client();
    try {
      uniquepair_client->find(request_metadata, "follow", follower_id,
          followee_id);
      follow_exists = true;
    }
    catch (TUniquepairNotFoundException e) {
      follow_exists = false;
    }
    uniquepair_client->close();
    return follow_exists;
  }

  int32_t count_followers(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("follow");
    query.__set_second_elem(account_id);

    // Count unique pairs.
    auto uniquepair_client = get_uniquepair_client();
    auto count = uniquepair_client->count(request_metadata, query);
    uniquepair_client->close();
    return count;
  }

  int32_t count_followees(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    // Build query struct.
    TUniquepairQuery query;
    query.__set_domain("follow");
    query.__set_first_elem(account_id);

    // Count unique pairs.
    auto uniquepair_client = get_uniquepair_client();
    auto count = uniquepair_client->count(request_metadata, query);
    uniquepair_client->close();
    return count;
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("follow_server", "Follow server");
  options.add_options()
      ("host", "", cxxopts::value<std::string>()->default_value("0.0.0.0"))
      ("port", "", cxxopts::value<int>())
      ("threads", "", cxxopts::value<int>())
      ("backend_filepath", "", cxxopts::value<std::string>()->default_value(
          "/etc/opt/BuzzBlogApp/backend.yml"))
      ("postgres_user", "", cxxopts::value<std::string>()->default_value(
          "postgres"))
      ("postgres_password", "", cxxopts::value<std::string>()->default_value(
          "postgres"))
      ("postgres_dbname", "", cxxopts::value<std::string>()->default_value(
          "postgres"));

  // Parse command-line arguments.
  auto result = options.parse(argc, argv);
  std::string host = result["host"].as<std::string>();
  int port = result["port"].as<int>();
  int threads = result["threads"].as<int>();
  std::string backend_filepath = result["backend_filepath"].as<std::string>();
  std::string postgres_user = result["postgres_user"].as<std::string>();
  std::string postgres_password = result["postgres_password"].as<std::string>();
  std::string postgres_dbname = result["postgres_dbname"].as<std::string>();

  // Initialize logger.
  auto logger = spdlog::basic_logger_mt("logger", "/tmp/calls.log");
  logger->set_pattern("[%H:%M:%S.%F] pid=%P tid=%t %v");

  // Create server.
  TThreadedServer server(
      std::make_shared<TFollowServiceProcessor>(
          std::make_shared<TFollowServiceHandler>(backend_filepath,
              postgres_user, postgres_password, postgres_dbname)),
      std::make_shared<TServerSocket>(host, port),
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
