#include "httplib.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static const std::string DATA_DIR = "/data/blocks";

int main() {
    fs::create_directories(DATA_DIR);

    httplib::Server server;

    // 🔹 health
    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK\n", "text/plain");
    });

    // 🔹 PUT /block/{id}
    server.Put(R"(/block/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string block_id = req.matches[1];
        std::string file_path = DATA_DIR + "/" + block_id;

        std::ofstream out(file_path, std::ios::binary);
        if (!out) {
            res.status = 500;
            res.set_content("failed to open file\n", "text/plain");
            return;
        }

        out.write(req.body.data(), req.body.size());
        out.close();

        std::cout << "Stored block: " << block_id
                  << " size=" << req.body.size() << "\n";

        res.set_content("OK\n", "text/plain");
    });

    // 🔹 GET /block/{id}
    server.Get(R"(/block/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string block_id = req.matches[1];
        std::string file_path = DATA_DIR + "/" + block_id;

        std::ifstream in(file_path, std::ios::binary);
        if (!in) {
            res.status = 404;
            res.set_content("block not found\n", "text/plain");
            return;
        }

        // ✅ FIX: używamy {} zamiast ()
        std::string data{
            std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>()
        };

        std::cout << "Read block: " << block_id
                  << " size=" << data.size() << "\n";

        res.set_content(data, "application/octet-stream");
    });

    std::cout << "storage_node listening on 0.0.0.0:8080\n";
    server.listen("0.0.0.0", 8080);

    return 0;
}