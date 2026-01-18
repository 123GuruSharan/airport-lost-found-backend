#define CROW_DISABLE_BOOST
#include "crow_all.h"

#include <vector>
#include <list>
#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

struct Item {
    int id;
    string description;
    string location;
    string date;
    bool isLost;

    Item(int i, const string& desc, const string& loc, const string& d, bool lost)
        : id(i), description(desc), location(loc), date(d), isLost(lost) {}
};

class HashTable {
private:
    static const int TABLE_SIZE = 10;
    vector<list<Item>> table;

    int hashFunction(int key) {
        return key % TABLE_SIZE;
    }

public:
    HashTable() : table(TABLE_SIZE) {}

    void insertItem(const Item& item) {
        int index = hashFunction(item.id);
        table[index].push_back(item);
    }

    vector<Item> getAll() {
        vector<Item> results;
        for (auto& chain : table) {
            for (auto& item : chain) {
                results.push_back(item);
            }
        }
        return results;
    }

    vector<Item> searchById(int id) {
        int index = hashFunction(id);
        vector<Item> results;
        for (auto& item : table[index]) {
            if (item.id == id)
                results.push_back(item);
        }
        return results;
    }
};

HashTable tracker;

int main() {
    crow::SimpleApp app;

    // 🔹 Health check
    CROW_ROUTE(app, "/")([](){
        return crow::response(200, "Backend is running!");
    });

    // 🔹 ADD ITEM (POST)
    CROW_ROUTE(app, "/add").methods(crow::HTTPMethod::Post)
    ([&](const crow::request& req){
        std::cout << "[INFO] /add called\n";

        auto x = crow::json::load(req.body);
        if (!x) {
            std::cout << "[ERROR] Invalid JSON\n";
            return crow::response(400, R"({"error":"Invalid JSON body"})");
        }

        if (!x.has("id") || !x.has("description") || !x.has("location")
            || !x.has("date") || !x.has("isLost")) {
            std::cout << "[ERROR] Missing fields\n";
            return crow::response(400, R"({"error":"Missing required fields"})");
        }

        tracker.insertItem(Item(
            x["id"].i(),
            x["description"].s(),
            x["location"].s(),
            x["date"].s(),
            x["isLost"].b()
        ));

        std::cout << "[SUCCESS] Item added\n";
        return crow::response(200, R"({"message":"Item added successfully"})");
    });

    // 🔹 GET ALL ITEMS
    CROW_ROUTE(app, "/items")
    ([]{
        auto items = tracker.getAll();
        crow::json::wvalue result;

        int i = 0;
        for (auto& item : items) {
            result[i]["id"] = item.id;
            result[i]["description"] = item.description;
            result[i]["location"] = item.location;
            result[i]["date"] = item.date;
            result[i]["isLost"] = item.isLost;
            i++;
        }

        return result;
    });

    // 🔹 SEARCH BY ID
    CROW_ROUTE(app, "/search/<int>")
    ([](int id){
        auto results = tracker.searchById(id);
        crow::json::wvalue response;

        if (results.empty()) {
            response["message"] = "No item found for given ID";
            response["results"] = crow::json::wvalue::list();
            return response;
        }

        int i = 0;
        for (auto& item : results) {
            response["results"][i]["id"] = item.id;
            response["results"][i]["description"] = item.description;
            response["results"][i]["location"] = item.location;
            response["results"][i]["date"] = item.date;
            response["results"][i]["isLost"] = item.isLost;
            i++;
        }

        return response;
    });

    // 🔹 PORT (Cloud safe)
    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::stoi(portEnv) : 8080;

    std::cout << "[INFO] Server starting on port " << port << "\n";
    app.port(port).multithreaded().run();
}
