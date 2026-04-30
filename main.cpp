#include <fstream>
#include "third_party/httplib.h"

int main() {
    httplib::Server svr;

    // PUT /block/<name>
    svr.Put(R"(/block/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string name = req.matches[1];
        std::ofstream out("data/" + name);
        out << req.body;
        res.set_content("OK\n", "text/plain");
    });

    // GET /block/<name>
    svr.Get(R"(/block/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string name = req.matches[1];
        std::ifstream in("data/" + name);
        std::string content((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
        res.set_content(content, "text/plain");
    });

    svr.listen("0.0.0.0", 9001);
}