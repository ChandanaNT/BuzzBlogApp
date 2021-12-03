// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <string>

#include <cxxopts.hpp>
#include <pqxx/pqxx>
#include <spdlog/sinks/basic_file_sink.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include <buzzblog/gen/TAccountService.h>
#include <buzzblog/base_server.h>


using apache::thrift;
using apache::thrift::protocol;
using apache::thrift::transport;
using apache::thrift::server;

using gen;


class TAccountServiceHandler : public BaseServer, public TAccountServiceIf {
 private:
  bool validate_attributes(const std::string& username,
      const std::string& password, const std::string& first_name,
      const std::string& last_name) {
    return (username.size() > 0 && username.size() <= 32 &&
        password.size() > 0 && password.size() <= 32 &&
        first_name.size() > 0 && first_name.size() <= 32 &&
        last_name.size() > 0 && last_name.size() <= 32);
  }

 public:
  TAccountServiceHandler(const std::string& backend_filepath,
      const std::string& postgres_user, const std::string& postgres_password,
      const std::string& postgres_dbname)
  : BaseServer(backend_filepath, postgres_user, postgres_password,
      postgres_dbname) {
  }

  void authenticate_user(TAccount& _return,
      const TRequestMetadata& request_metadata, const std::string& username,
      const std::string& password) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT id, created_at, active, password, first_name, last_name "
        "FROM Accounts "
        "WHERE username = '%s'";
    snprintf(query_str, sizeof(query_str), query_fmt, username.c_str());

    // Execute query.
    pqxx::connection conn(account_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountInvalidCredentialsException();

    // Check if account is active.
    if (db_res[0][2].as<bool>() == false)
      throw TAccountDeactivatedException();

    // Check if password is correct.
    if (password != db_res[0][3].as<std::string>())
      throw TAccountInvalidCredentialsException();

    // Build account (standard mode).
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.active = true;
    _return.username = username;
    _return.first_name = db_res[0][4].as<std::string>();
    _return.last_name = db_res[0][5].as<std::string>();
  }

  void create_account(TAccount& _return,
      const TRequestMetadata& request_metadata, const std::string& username,
      const std::string& password, const std::string& first_name,
      const std::string& last_name) {
    // Validate attributes.
    if (!validate_attributes(username, password, first_name, last_name))
      throw TAccountInvalidAttributesException();

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "INSERT INTO Accounts (created_at, username, password, first_name, "
            "last_name) "
        "VALUES (extract(epoch from now()), '%s', '%s', '%s', '%s') "
        "RETURNING id, created_at";
    snprintf(query_str, sizeof(query_str), query_fmt, username.c_str(),
               password.c_str(), first_name.c_str(), last_name.c_str());

    // Execute query.
    pqxx::connection conn(account_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res;
    try {
      db_res = txn.exec(query_str);
    }
    catch (pqxx::sql_error& e) {
      throw TAccountUsernameAlreadyExistsException();
    }
    txn.commit();
    conn.disconnect();

    // Build account (standard mode).
    _return.id = db_res[0][0].as<int>();
    _return.created_at = db_res[0][1].as<int>();
    _return.active = true;
    _return.username = username;
    _return.first_name = first_name;
    _return.last_name = last_name;
  }

  void retrieve_standard_account(TAccount& _return,
      const TRequestMetadata& request_metadata, int32_t account_id) {
    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "SELECT created_at, active, username, first_name, last_name "
        "FROM Accounts "
        "WHERE id = %d";
    snprintf(query_str, sizeof(query_str), query_fmt, account_id);

    // Execute query.
    pqxx::connection conn(account_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountNotFoundException();

    // Build account (standard mode).
    _return.id = account_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.active = db_res[0][1].as<bool>();
    _return.username = db_res[0][2].as<std::string>();
    _return.first_name = db_res[0][3].as<std::string>();
    _return.last_name = db_res[0][4].as<std::string>();
  }

  void retrieve_expanded_account(TAccount& _return,
      const TRequestMetadata& request_metadata, int32_t account_id) {
    // Retrieve standard account.
    retrieve_standard_account(_return, request_metadata, account_id);

    // Retrieve follow activity.
    auto follow_client = get_follow_client();
    auto follows_you = follow_client->check_follow(request_metadata, account_id,
        request_metadata.requester_id);
    auto followed_by_you = follow_client->check_follow(request_metadata,
        request_metadata.requester_id, account_id);
    auto n_followers = follow_client->count_followers(request_metadata,
        account_id);
    auto n_following = follow_client->count_followees(request_metadata,
        account_id);
    follow_client->close();

    // Retrieve post activity.
    auto post_client = get_post_client();
    auto n_posts = post_client->count_posts_by_author(request_metadata,
        account_id);
    post_client->close();

    // Retrieve like activity.
    auto like_client = get_like_client();
    auto n_likes = like_client->count_likes_by_account(request_metadata,
        account_id);
    like_client->close();

    // Build account (expanded mode).
    _return.__set_follows_you(follows_you);
    _return.__set_followed_by_you(followed_by_you);
    _return.__set_n_followers(n_followers);
    _return.__set_n_following(n_following);
    _return.__set_n_posts(n_posts);
    _return.__set_n_likes(n_likes);
  }

  void update_account(TAccount& _return,
      const TRequestMetadata& request_metadata, const int32_t account_id,
      const std::string& password, const std::string& first_name,
      const std::string& last_name) {
    // Check if requester is authorized.
    if (request_metadata.requester_id != account_id)
      throw TAccountNotAuthorizedException();

    // Validate attributes.
    // NOTE: "john.doe" is a valid username. It is used because the actual
    // username is unknown and will not be updated anyway.
    if (!validate_attributes("john.doe", password, first_name, last_name))
      throw TAccountInvalidAttributesException();

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "UPDATE Accounts "
        "SET password = '%s', first_name = '%s', last_name = '%s' "
        "WHERE id = %d "
        "RETURNING created_at, active, username";
    snprintf(query_str, sizeof(query_str), query_fmt, password.c_str(),
              first_name.c_str(), last_name.c_str(), account_id);

    // Execute query.
    pqxx::connection conn(account_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountNotFoundException();

    // Build account (standard mode).
    _return.id = account_id;
    _return.created_at = db_res[0][0].as<int>();
    _return.active = db_res[0][1].as<bool>();
    _return.username = db_res[0][2].as<std::string>();
    _return.first_name = first_name;
    _return.last_name = last_name;
  }

  void delete_account(const TRequestMetadata& request_metadata,
      const int32_t account_id) {
    // Check if requester is authorized.
    if (request_metadata.requester_id != account_id)
      throw TAccountNotAuthorizedException();

    // Build query string.
    char query_str[1024];
    const char *query_fmt = \
        "UPDATE Accounts "
        "SET active = FALSE "
        "WHERE id = %d "
        "RETURNING id";
    snprintf(query_str, sizeof(query_str), query_fmt, account_id);

    // Execute query.
    pqxx::connection conn(account_db_conn_str);
    pqxx::work txn(conn);
    pqxx::result db_res(txn.exec(query_str));
    txn.commit();
    conn.disconnect();

    // Check if account exists.
    if (db_res.begin() == db_res.end())
      throw TAccountNotFoundException();
  }
};


int main(int argc, char** argv) {
  // Define command-line parameters.
  cxxopts::Options options("account_server", "Account server");
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
      std::make_shared<TAccountServiceProcessor>(
          std::make_shared<TAccountServiceHandler>(backend_filepath,
              postgres_user, postgres_password, postgres_dbname)),
      std::make_shared<TServerSocket>(host, port),
      std::make_shared<TBufferedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  server.setConcurrentClientLimit(threads);

  // Serve requests.
  server.serve();

  return 0;
}
