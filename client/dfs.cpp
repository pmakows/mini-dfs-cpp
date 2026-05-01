#include "httplib.h"
#include "json.hpp"

#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

static const std::string METADATA_HOST = "localhost";
static const int METADATA_PORT = 9000;

static const std::string STORAGE_HOST = "localhost";
static const int STORAGE_PORT = 9001;

static const size_t CHUNK_SIZE = 64 * 1024;

std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open input file: " + path);
    }

    return std::string(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    );
}

void write_file(const std::string& path, const std::string& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("cannot open output file: " + path);
    }

    out.write(data.data(), data.size());
}

void put_file(const std::string& local_path, const std::string& remote_path) {
    std::string data = read_file(local_path);

    // Na razie jeden blok. Chunking zrobimy w M4.
    std::string block_id = "block_" + std::to_string(std::hash<std::string>{}(remote_path));

    httplib::Client storage(STORAGE_HOST, STORAGE_PORT);

    auto storage_res = storage.Put(
        ("/block/" + block_id).c_str(),
        data,
        "application/octet-stream"
    );

    if (!storage_res || storage_res->status != 200) {
        throw std::runtime_error("failed to store block");
    }

    json metadata_json = {
        {"path", remote_path},
        {"blocks", json::array({
            {
                {"id", block_id},
                {"nodes", json::array({"storage1:8080"})},
                {"size", data.size()}
            }
        })}
    };

    httplib::Client metadata(METADATA_HOST, METADATA_PORT);

    auto metadata_res = metadata.Post(
        "/files",
        metadata_json.dump(),
        "application/json"
    );

    if (!metadata_res || metadata_res->status != 200) {
        throw std::runtime_error("failed to store metadata");
    }

    std::cout << "PUT OK: " << local_path << " -> " << remote_path << "\n";
}

void get_file(const std::string& remote_path, const std::string& local_path) {
    httplib::Client metadata(METADATA_HOST, METADATA_PORT);

    auto metadata_res = metadata.Get(("/files" + remote_path).c_str());

    if (!metadata_res || metadata_res->status != 200) {
        throw std::runtime_error("failed to get metadata");
    }

    json metadata_json = json::parse(metadata_res->body);

    std::string block_id = metadata_json["blocks"][0]["id"];

    httplib::Client storage(STORAGE_HOST, STORAGE_PORT);

    auto storage_res = storage.Get(("/block/" + block_id).c_str());

    if (!storage_res || storage_res->status != 200) {
        throw std::runtime_error("failed to get block");
    }

    write_file(local_path, storage_res->body);

    std::cout << "GET OK: " << remote_path << " -> " << local_path << "\n";
}

std::vector<std::string> split_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open file");
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

int main(int argc, char** argv) {
    try {
        // DEBUG MODE: split
        if (argc == 3 && std::string(argv[1]) == "split") {
            auto chunks = split_file(argv[2]);

            std::cout << "Chunks: " << chunks.size() << "\n";

            for (size_t i = 0; i < chunks.size(); ++i) {
                std::cout << "Chunk " << i
                          << " size=" << chunks[i].size() << "\n";
            }

            return 0;
        }
        // end of DEBUG MODE

        if (argc != 4) {
            std::cerr << "usage:\n";
            std::cerr << "  dfs put <local_path> <remote_path>\n";
            std::cerr << "  dfs get <remote_path> <local_path>\n";
            std::cerr << "  dfs split <local_path>\n"; // 👈 dodajemy do helpa
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