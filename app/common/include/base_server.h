// Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
// Systems

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <buzzblog/account_client.h>
#include <buzzblog/follow_client.h>
#include <buzzblog/like_client.h>
#include <buzzblog/post_client.h>
#include <buzzblog/uniquepair_client.h>


class BaseServer {
 protected:
  BaseServer(const std::string& backend_filepath,
      const std::string& postgres_user,
      const std::string& postgres_password,
      const std::string& postgres_dbname) {
    char conn_cstr[128];
    const char *conn_fmt = "postgres://%s:%s@%s:%d/%s";

    // Parse configuration.
    std::cout << "Initializing BaseServer:" << std::endl;
    auto backend = YAML::LoadFile(backend_filepath);
    if (backend["account"]) {
      // Load account service configuration.
      auto account_service = backend["account"]["service"];
      for (auto it = account_service.begin(); it != account_service.end();
           it++) {
        auto server = it->as<std::string>();
        auto hostname = server.substr(0, server.find(":"));
        auto port = std::stoi(server.substr(server.find(":") + 1));
        this->account_service.push_back(std::make_pair(hostname, port));
        std::cout << "\tAdded account service on " << \
            hostname << ":" << port << std::endl;
      }
      // Build account database connection string.
      auto account_db = backend["account"]["database"].as<std::string>();
      auto account_db_host = account_db.substr(0, account_db.find(":"));
      auto account_db_port = std::stoi(
          account_db.substr(account_db.find(":") + 1));
      snprintf(conn_cstr, sizeof(conn_cstr), conn_fmt, postgres_user.c_str(),
          postgres_password.c_str(), account_db_host.c_str(), account_db_port,
          postgres_dbname.c_str());
      account_db_conn_str = std::string(conn_cstr);
      std::cout << "\tAdded account database on: " << \
          account_db_host << ":" << account_db_port << std::endl;
    }
    if (backend["follow"]) {
      // Load follow service configuration.
      auto follow_service = backend["follow"]["service"];
      for (auto it = follow_service.begin(); it != follow_service.end(); it++) {
        auto server = it->as<std::string>();
        auto hostname = server.substr(0, server.find(":"));
        auto port = std::stoi(server.substr(server.find(":") + 1));
        this->follow_service.push_back(std::make_pair(hostname, port));
        std::cout << "\tAdded follow service on " << \
            hostname << ":" << port << std::endl;
      }
    }
    if (backend["like"]) {
      // Load like service configuration.
      auto like_service = backend["like"]["service"];
      for (auto it = like_service.begin(); it != like_service.end(); it++) {
        auto server = it->as<std::string>();
        auto hostname = server.substr(0, server.find(":"));
        auto port = std::stoi(server.substr(server.find(":") + 1));
        this->like_service.push_back(std::make_pair(hostname, port));
        std::cout << "\tAdded like service on " << \
            hostname << ":" << port << std::endl;
      }
    }
    if (backend["post"]) {
      // Load post service configuration.
      auto post_service = backend["post"]["service"];
      for (auto it = post_service.begin(); it != post_service.end(); it++) {
        auto server = it->as<std::string>();
        auto hostname = server.substr(0, server.find(":"));
        auto port = std::stoi(server.substr(server.find(":") + 1));
        this->post_service.push_back(std::make_pair(hostname, port));
        std::cout << "\tAdded post service on " << \
            hostname << ":" << port << std::endl;
      }
      // Build post database connection string.
      auto post_db = backend["post"]["database"].as<std::string>();
      auto post_db_host = post_db.substr(0, post_db.find(":"));
      auto post_db_port = std::stoi(post_db.substr(post_db.find(":") + 1));
      snprintf(conn_cstr, sizeof(conn_cstr), conn_fmt, postgres_user.c_str(),
          postgres_password.c_str(), post_db_host.c_str(), post_db_port,
          postgres_dbname.c_str());
      post_db_conn_str = std::string(conn_cstr);
      std::cout << "\tAdded post database on: " << \
          post_db_host << ":" << post_db_port << std::endl;
    }
    if (backend["uniquepair"]) {
      // Load uniquepair service configuration.
      auto uniquepair_service = backend["uniquepair"]["service"];
      for (auto it = uniquepair_service.begin(); it != uniquepair_service.end();
          it++) {
        auto server = it->as<std::string>();
        auto hostname = server.substr(0, server.find(":"));
        auto port = std::stoi(server.substr(server.find(":") + 1));
        this->uniquepair_service.push_back(std::make_pair(hostname, port));
        std::cout << "\tAdded uniquepair service on " << \
            hostname << ":" << port << std::endl;
      }
      // Build uniquepair database connection string.
      auto uniquepair_db = backend["uniquepair"]["database"].as<std::string>();
      auto uniquepair_db_host = uniquepair_db.substr(0,
                                uniquepair_db.find(":"));
      auto uniquepair_db_port = std::stoi(
          uniquepair_db.substr(uniquepair_db.find(":") + 1));
      snprintf(conn_cstr, sizeof(conn_cstr), conn_fmt, postgres_user.c_str(),
          postgres_password.c_str(), uniquepair_db_host.c_str(),
          uniquepair_db_port, postgres_dbname.c_str());
      uniquepair_db_conn_str = std::string(conn_cstr);
      std::cout << "\tAdded uniquepair database on: " << \
          uniquepair_db_host << ":" << uniquepair_db_port << std::endl;
    }
  }

  std::unique_ptr<account_service::Client> get_account_client() {
    // Randomly select a server.
    std::pair<std::string, int> server =
        account_service[rand() % static_cast<int>(account_service.size())];
    return std::move(std::make_unique<account_service::Client>(
        server.first, server.second, 10000));
  }

  std::unique_ptr<follow_service::Client> get_follow_client() {
    // Randomly select a server.
    std::pair<std::string, int> server =
        follow_service[rand() % static_cast<int>(follow_service.size())];
    return std::move(std::make_unique<follow_service::Client>(
        server.first, server.second, 10000));
  }

  std::unique_ptr<like_service::Client> get_like_client() {
    // Randomly select a server.
    std::pair<std::string, int> server =
        like_service[rand() % static_cast<int>(like_service.size())];
    return std::move(std::make_unique<like_service::Client>(
        server.first, server.second, 10000));
  }

  std::unique_ptr<post_service::Client> get_post_client() {
    // Randomly select a server.
    std::pair<std::string, int> server =
        post_service[rand() % static_cast<int>(post_service.size())];
    return std::move(std::make_unique<post_service::Client>(
        server.first, server.second, 10000));
  }

  std::unique_ptr<uniquepair_service::Client> get_uniquepair_client() {
    // Randomly select a server.
    std::pair<std::string, int> server =
        uniquepair_service[rand() % static_cast<int>(
                           uniquepair_service.size())];
    return std::move(std::make_unique<uniquepair_service::Client>(
        server.first, server.second, 10000));
  }

  // Pairs of server hosts and ports.
  std::vector<std::pair<std::string, int>> account_service;
  std::vector<std::pair<std::string, int>> follow_service;
  std::vector<std::pair<std::string, int>> like_service;
  std::vector<std::pair<std::string, int>> post_service;
  std::vector<std::pair<std::string, int>> uniquepair_service;
  // Database connection strings.
  std::string account_db_conn_str;
  std::string post_db_conn_str;
  std::string uniquepair_db_conn_str;
};
