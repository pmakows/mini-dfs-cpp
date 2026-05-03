#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <functional>

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"

using json = nlohmann::json;

static const std::string METADATA_HOST = "localhost";
static const int METADATA_PORT = 9000;

static const std::string CACHE_HOST = "localhost";
static const int CACHE_PORT = 9100;

static const size_t CHUNK_SIZE = 64 * 1024;
static const int REPLICATION = 2;

static const std::vector<std::string> STORAGE_NODES = {
    "localhost:9001",
    "localhost:9002",
    "localhost:9003"
};

static std::string normalize_remote_path(std::string path) {
    if (path.empty()) {
        return "/";
    }
    if (path[0] != '/') {
        path = "/" + path;
    }
    return path;
}

static std::string strip_leading_slash(std::string path) {
    while (!path.empty() && path[0] == '/') {
        path.erase(0, 1);
    }
    return path;
}

static bool read_file(const std::string& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return true;
}

static bool write_file(const std::string& path, const std::string& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    return true;
}

static std::string make_block_id(const std::string& remote_path, int index) {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::string input = remote_path + "_" + std::to_string(index) + "_" + std::to_string(now);
    return "block_" + std::to_string(std::hash<std::string>{}(input));
}

static bool cache_get(const std::string& remote_path, std::string& data) {
    httplib::Client cache(CACHE_HOST, CACHE_PORT);

    std::string key = strip_leading_slash(remote_path);
    auto res = cache.Get(("/cache/" + key).c_str());

    if (res && res->status == 200) {
        data = res->body;
        std::cout << "[CLIENT] CACHE HIT: " << key << std::endl;
        return true;
    }

    std::cout << "[CLIENT] CACHE MISS: " << key << std::endl;
    return false;
}

static void cache_put(const std::string& remote_path, const std::string& data) {
    httplib::Client cache(CACHE_HOST, CACHE_PORT);

    std::string key = strip_leading_slash(remote_path);
    auto res = cache.Put(
        ("/cache/" + key).c_str(),
        data,
        "application/octet-stream"
    );

    if (res && res->status == 200) {
        std::cout << "[CLIENT] CACHE STORE: " << key << std::endl;
    }
}

static void cache_delete(const std::string& remote_path) {
    httplib::Client cache(CACHE_HOST, CACHE_PORT);

    std::string key = strip_leading_slash(remote_path);
    auto res = cache.Delete(("/cache/" + key).c_str());

    if (res && res->status == 200) {
        std::cout << "[CLIENT] CACHE INVALIDATE: " << key << std::endl;
    }
}

static bool put_file(const std::string& local_path, const std::string& remote_path_raw) {
    std::string remote_path = normalize_remote_path(remote_path_raw);

    std::string data;
    if (!read_file(local_path, data)) {
        std::cerr << "error: cannot read local file: " << local_path << std::endl;
        return false;
    }

    json metadata;
    metadata["path"] = remote_path;
    metadata["blocks"] = json::array();

    size_t offset = 0;
    int block_index = 0;

    while (offset < data.size() || (data.empty() && block_index == 0)) {
        size_t remaining = data.size() - offset;
        size_t current_size = std::min(CHUNK_SIZE, remaining);

        std::string chunk = data.substr(offset, current_size);
        std::string block_id = make_block_id(remote_path, block_index);

        json block;
        block["id"] = block_id;
        block["size"] = chunk.size();
        block["nodes"] = json::array();

        int replicas_written = 0;

        for (size_t i = 0; i < STORAGE_NODES.size() && replicas_written < REPLICATION; ++i) {
            size_t node_index = (static_cast<size_t>(block_index) + i) % STORAGE_NODES.size();
            std::string node = STORAGE_NODES[node_index];

            auto colon = node.find(':');
            std::string host = node.substr(0, colon);
            int port = std::stoi(node.substr(colon + 1));

            httplib::Client storage(host, port);
            auto storage_res = storage.Put(
                ("/block/" + block_id).c_str(),
                chunk,
                "application/octet-stream"
            );

            if (storage_res && storage_res->status == 200) {
                block["nodes"].push_back(node);
                replicas_written++;

                std::cout << "Stored chunk " << block_index
                          << " id=" << block_id
                          << " size=" << chunk.size()
                          << " on " << node << std::endl;
            } else {
                std::cerr << "warning: failed to store chunk "
                          << block_index << " on " << node << std::endl;
            }
        }

        if (block["nodes"].size() < REPLICATION) {
            std::cerr << "error: insufficient replicas for block "
                      << block_id << " (have "
                      << block["nodes"].size()
                      << ", need " << REPLICATION << ")"
                      << std::endl;
            return false;
        }

        metadata["blocks"].push_back(block);

        offset += current_size;
        block_index++;

        if (data.empty()) {
            break;
        }
    }

    httplib::Client metadata_client(METADATA_HOST, METADATA_PORT);
    auto metadata_res = metadata_client.Post(
        "/files",
        metadata.dump(),
        "application/json"
    );

    if (!metadata_res || metadata_res->status != 200) {
        std::cerr << "error: failed to store metadata" << std::endl;
        return false;
    }

    cache_delete(remote_path);

    if (data.size() <= CHUNK_SIZE) {
        cache_put(remote_path, data);
    }

    std::cout << "PUT OK: " << local_path
              << " -> " << strip_leading_slash(remote_path)
              << " (" << metadata["blocks"].size()
              << " blocks, replication=" << REPLICATION << ")"
              << std::endl;

    return true;
}

static bool get_file(const std::string& remote_path_raw, const std::string& local_path) {
    std::string remote_path = normalize_remote_path(remote_path_raw);

    std::string cached;
    if (cache_get(remote_path, cached)) {
        if (!write_file(local_path, cached)) {
            std::cerr << "error: cannot write output file: " << local_path << std::endl;
            return false;
        }

        std::cout << "GET OK: " << strip_leading_slash(remote_path)
                  << " -> " << local_path << std::endl;
        return true;
    }

    httplib::Client metadata_client(METADATA_HOST, METADATA_PORT);
    std::string metadata_url = "/files/" + strip_leading_slash(remote_path);

    auto metadata_res = metadata_client.Get(metadata_url.c_str());

    if (!metadata_res || metadata_res->status != 200) {
        std::cerr << "error: failed to get metadata" << std::endl;
        return false;
    }

    json metadata = json::parse(metadata_res->body);

    std::string output;

    for (const auto& block : metadata["blocks"]) {
        std::string block_id = block["id"];
        bool block_read = false;

        for (const auto& node_json : block["nodes"]) {
            std::string node = node_json.get<std::string>();

            auto colon = node.find(':');
            std::string host = node.substr(0, colon);
            int port = std::stoi(node.substr(colon + 1));

            httplib::Client storage(host, port);
            auto storage_res = storage.Get(("/block/" + block_id).c_str());

            if (storage_res && storage_res->status == 200) {
                output += storage_res->body;
                block_read = true;
                break;
            }

            std::cerr << "warning: failed to read block "
                      << block_id << " from " << node
                      << ", trying next replica" << std::endl;
        }

        if (!block_read) {
            std::cerr << "error: failed to read block from all replicas: "
                      << block_id << std::endl;
            return false;
        }
    }

    if (!write_file(local_path, output)) {
        std::cerr << "error: cannot write output file: " << local_path << std::endl;
        return false;
    }

    if (output.size() <= CHUNK_SIZE) {
        cache_put(remote_path, output);
    }

    std::cout << "GET OK: " << strip_leading_slash(remote_path)
              << " -> " << local_path << std::endl;

    return true;
}

static void print_usage() {
    std::cerr << "usage:\n";
    std::cerr << "  dfs put <local_path> <remote_path>\n";
    std::cerr << "  dfs get <remote_path> <local_path>\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "put") {
        if (argc != 4) {
            print_usage();
            return 1;
        }

        return put_file(argv[2], argv[3]) ? 0 : 1;
    }

    if (command == "get") {
        if (argc != 4) {
            print_usage();
            return 1;
        }

        return get_file(argv[2], argv[3]) ? 0 : 1;
    }

    print_usage();
    return 1;
}