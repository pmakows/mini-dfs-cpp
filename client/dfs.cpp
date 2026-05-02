#include "httplib.h"
#include "json.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

static const std::string METADATA_HOST = "localhost";
static const int METADATA_PORT = 9000;

static const size_t CHUNK_SIZE = 64 * 1024;
static const int REPLICATION = 2;

static const std::vector<std::string> STORAGE_NODES = {
    "localhost:9001",
    "localhost:9002",
    "localhost:9003"
};

struct NodeAddress {
    std::string host;
    int port;
};

NodeAddress parse_node(const std::string& node) {
    auto pos = node.find(":");
    if (pos == std::string::npos) {
        throw std::runtime_error("invalid node address: " + node);
    }

    return {
        node.substr(0, pos),
        std::stoi(node.substr(pos + 1))
    };
}

std::vector<std::string> split_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open file: " + path);
    }

    std::vector<std::string> chunks;

    while (!in.eof()) {
        std::string buffer(CHUNK_SIZE, '\0');
        in.read(buffer.data(), CHUNK_SIZE);
        size_t bytes_read = in.gcount();

        if (bytes_read > 0) {
            buffer.resize(bytes_read);
            chunks.push_back(buffer);
        }
    }

    return chunks;
}

void put_file(const std::string& local_path, const std::string& remote_path) {
    auto chunks = split_file(local_path);

    httplib::Client metadata(METADATA_HOST, METADATA_PORT);
    json blocks = json::array();

    for (size_t i = 0; i < chunks.size(); ++i) {
        std::string block_id =
            "block_" + std::to_string(std::hash<std::string>{}(remote_path + std::to_string(i)));

        json nodes = json::array();

        for (int r = 0; r < REPLICATION; ++r) {
            size_t node_index = (i + r) % STORAGE_NODES.size();
            std::string node = STORAGE_NODES[node_index];

            NodeAddress addr = parse_node(node);
            httplib::Client storage(addr.host, addr.port);

            auto storage_res = storage.Put(
                ("/block/" + block_id).c_str(),
                chunks[i],
                "application/octet-stream"
            );

            if (storage_res && storage_res->status == 200) {
                nodes.push_back(node);

                std::cout << "Stored chunk " << i
                          << " id=" << block_id
                          << " size=" << chunks[i].size()
                          << " on " << node << "\n";
            }
        }

        if (nodes.empty()) {
            throw std::runtime_error("failed to store block on any node: " + block_id);
        }

        blocks.push_back({
            {"id", block_id},
            {"nodes", nodes},
            {"size", chunks[i].size()}
        });
    }

    json metadata_json = {
        {"path", remote_path},
        {"blocks", blocks}
    };

    auto metadata_res = metadata.Post(
        "/files",
        metadata_json.dump(),
        "application/json"
    );

    if (!metadata_res || metadata_res->status != 200) {
        throw std::runtime_error("failed to store metadata");
    }

    std::cout << "PUT OK: " << local_path << " -> " << remote_path
              << " (" << chunks.size() << " blocks, replication="
              << REPLICATION << ")\n";
}

void get_file(const std::string& remote_path, const std::string& local_path) {
    httplib::Client metadata(METADATA_HOST, METADATA_PORT);

    auto metadata_res = metadata.Get(("/files" + remote_path).c_str());

    if (!metadata_res || metadata_res->status != 200) {
        throw std::runtime_error("failed to get metadata");
    }

    json metadata_json = json::parse(metadata_res->body);

    std::ofstream out(local_path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("cannot open output file: " + local_path);
    }

    for (auto& block : metadata_json["blocks"]) {
        std::string block_id = block["id"];
        bool fetched = false;

        for (auto& node_json : block["nodes"]) {
            std::string node = node_json;

            NodeAddress addr = parse_node(node);
            httplib::Client storage(addr.host, addr.port);

            auto storage_res = storage.Get(("/block/" + block_id).c_str());

            if (storage_res && storage_res->status == 200) {
                out.write(storage_res->body.data(), storage_res->body.size());
                fetched = true;

                std::cout << "Fetched block " << block_id
                          << " from " << node
                          << " size=" << storage_res->body.size() << "\n";
                break;
            }

            std::cout << "Failed to fetch block " << block_id
                      << " from " << node << "\n";
        }

        if (!fetched) {
            throw std::runtime_error("failed to fetch block from all replicas: " + block_id);
        }
    }

    std::cout << "GET OK: " << remote_path << " -> " << local_path << "\n";
}

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string(argv[1]) == "split") {
            auto chunks = split_file(argv[2]);

            std::cout << "Chunks: " << chunks.size() << "\n";

            for (size_t i = 0; i < chunks.size(); ++i) {
                std::cout << "Chunk " << i
                          << " size=" << chunks[i].size() << "\n";
            }

            return 0;
        }

        if (argc != 4) {
            std::cerr << "usage:\n";
            std::cerr << "  dfs put <local_path> <remote_path>\n";
            std::cerr << "  dfs get <remote_path> <local_path>\n";
            std::cerr << "  dfs split <local_path>\n";
            return 1;
        }

        std::string command = argv[1];

        if (command == "put") {
            put_file(argv[2], argv[3]);
        } else if (command == "get") {
            get_file(argv[2], argv[3]);
        } else {
            std::cerr << "unknown command: " << command << "\n";
            return 1;
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}