#include "httplib.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "../third_party/httplib.h"

namespace fs = std::filesystem;

static std::string get_data_dir() {
    const char* env = std::getenv("STORAGE_DATA_DIR");
    return env ? std::string(env) : "/data/blocks";
}

static const std::string DATA_DIR = get_data_dir();

int main() {
    try {
        fs::create_directories(DATA_DIR);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: cannot create data dir: " << DATA_DIR << "\n";
        std::cerr << e.what() << "\n";
        return 1;
    }

    httplib::Server server;

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK\n", "text/plain");
    });

    server.Put(R"(/block/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string block_id = req.matches[1];
        std::string file_path = DATA_DIR + "/" + block_id;

        std::ofstream out(file_path, std::ios::binary);
        if (!out) {
            res.status = 500;
            res.set_content("failed to open file\n", "text/plain");
            return;
        }

        out.write(req.body.data(), static_cast<std::streamsize>(req.body.size()));
        out.close();

        std::cout << "Stored block: " << block_id
                  << " size=" << req.body.size() << "\n";

        res.set_content("OK\n", "text/plain");
    });

    server.Get(R"(/block/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string block_id = req.matches[1];
        std::string file_path = DATA_DIR + "/" + block_id;

        std::ifstream in(file_path, std::ios::binary);
        if (!in) {
            res.status = 404;
            res.set_content("block not found\n", "text/plain");
            return;
        }

        std::string data{
            std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>()
        };

        std::cout << "Read block: " << block_id
                  << " size=" << data.size() << "\n";

        res.set_content(data, "application/octet-stream");
    });

    std::cout << "storage_node listening on 0.0.0.0:8080\n";
    std::cout << "data dir: " << DATA_DIR << "\n";

    server.listen("0.0.0.0", 8080);

    return 0;
}