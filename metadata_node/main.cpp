#include "httplib.h"
#include "json.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

using json = nlohmann::json;
namespace fs = std::filesystem;

static const std::string DATA_DIR = "/data";
static const std::string META_FILE = "/data/metadata.json";

std::mutex g_mutex;
json g_files = json::object();

void load_metadata() {
    fs::create_directories(DATA_DIR);

    std::ifstream in(META_FILE);
    if (!in) {
        std::cout << "No metadata file yet, starting empty\n";
        return;
    }

    try {
        in >> g_files;
        std::cout << "Loaded metadata\n";
    } catch (...) {
        std::cerr << "Failed to parse metadata.json, starting empty\n";
        g_files = json::object();
    }
}

void save_metadata() {
    fs::create_directories(DATA_DIR);

    std::ofstream out(META_FILE);
    out << g_files.dump(2);
}

int main() {
    load_metadata();

    httplib::Server server;

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("OK\n", "text/plain");
    });

    server.Post("/files", [](const httplib::Request& req, httplib::Response& res) {
        json body;

        try {
            body = json::parse(req.body);
        } catch (...) {
            res.status = 400;
            res.set_content("invalid json\n", "text/plain");
            return;
        }

        if (!body.contains("path") || !body.contains("blocks")) {
            res.status = 400;
            res.set_content("missing path or blocks\n", "text/plain");
            return;
        }

        std::string path = body["path"];

        {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_files[path] = body;
            save_metadata();
        }

        std::cout << "Stored metadata for file: " << path << "\n";
        res.set_content("metadata stored\n", "text/plain");
    });

    server.Get(R"(/files/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string path = "/" + std::string(req.matches[1]);

        std::lock_guard<std::mutex> lock(g_mutex);

        if (!g_files.contains(path)) {
            res.status = 404;
            res.set_content("file not found\n", "text/plain");
            return;
        }

        res.set_content(g_files[path].dump(2), "application/json");
    });

    server.Get("/files", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);
        res.set_content(g_files.dump(2), "application/json");
    });

    std::cout << "metadata_service listening on 0.0.0.0:8080\n";
    server.listen("0.0.0.0", 8080);

    return 0;
}