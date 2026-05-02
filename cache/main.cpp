#include <iostream>
#include "../third_party/httplib.h"

extern "C" {
#include "cache.h"
}

static cache_t g_cache;

int main()
{
    cache_init(&g_cache);

    httplib::Server server;

    server.Get("/cache/(.+)", [](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];

        char buffer[CACHE_VALUE_SIZE];
        size_t len = 0;

        if (cache_get(&g_cache, key.c_str(), buffer, sizeof(buffer), &len)) {
            std::cout << "[CACHE HIT] key=" << key << std::endl;
            res.set_content(std::string(buffer, len), "application/octet-stream");
            return;
        }

        std::cout << "[CACHE MISS] key=" << key << std::endl;
        res.status = 404;
    });

    server.Put("/cache/(.+)", [](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];

        cache_put(&g_cache, key.c_str(), req.body.data(), req.body.size());

        std::cout << "[CACHE PUT] key=" << key
                  << " size=" << req.body.size() << std::endl;

        res.status = 200;
        res.set_content("cached\n", "text/plain");
    });

    server.Delete("/cache/(.+)", [](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];

        cache_invalidate(&g_cache, key.c_str());

        std::cout << "[CACHE INVALIDATE] key=" << key << std::endl;

        res.status = 200;
        res.set_content("invalidated\n", "text/plain");
    });

    std::cout << "Cache node listening on port 9100" << std::endl;
    server.listen("0.0.0.0", 9100);

    return 0;
}