#define CROW_DISABLE_BOOST
#include "crow_all.h"

#include <vector>
#include <list>
#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

// ===================== DATA MODEL =====================
struct Item {
    int id;
    string description;
    string location;
    string date;
    bool isLost;

    Item(int i, const string& desc, const string& loc, const string& d, bool lost)
        : id(i), description(desc), location(loc), date(d), isLost(lost) {}
};

// ===================== STORAGE =====================
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
        table[hashFunction(item.id)].push_back(item);
    }

    vector<Item> getAll() {
        vector<Item> results;
        for (auto& bucket : table)
            for (auto& item : bucket)
                results.push_back(item);
        return results;
    }

    vector<Item> searchById(int id) {
        vector<Item> results;
        for (auto& item : table[hashFunction(id)])
            if (item.id == id)
                results.push_back(item);
        return results;
    }
};

HashTable tracker;

// ===================== MAIN =====================
int main() {
    crow::SimpleApp app;

    // ---------- GLOBAL CORS (for all routes) ----------
    app.before_handle([](crow::request&, crow::response& res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
    });

    // ---------- HEALTH CHECK ----------
    CROW_ROUTE(app, "/")([](){
        return crow::response(200, "Backend is running!");
    });

    // ---------- ADD ITEM (POST + OPTIONS) ----------
    CROW_ROUTE(app, "/add")
        .methods(crow::HTTPMethod::Post, crow::HTTPMethod::Options)
    ([](const crow::request& req){
        // CORS preflight
        if (req.method == crow::HTTPMethod::Options) {
            return crow::response(204);
        }

        auto x = crow::json::load(req.body);
        if (!x)
            return crow::response(400, "Invalid JSON");

        if (!x.has("id") || !x.has("description") || !x.has("location")
            || !x.has("date") || !x.has("isLost")) {
            return crow::response(400, "Missing required fields");
        }

        tracker.insertItem(Item(
            x["id"].i(),
            x["description"].s(),
            x["location"].s(),
            x["date"].s(),
            x["isLost"].b()
        ));

        return crow::response(200, "Item added successfully");
    });

    // ---------- GET ALL ITEMS ----------
    CROW_ROUTE(app, "/items")
    ([]{
        auto items = tracker.getAll();
        crow::json::wvalue res;

        int i = 0;
        for (auto& item : items) {
            res[i]["id"] = item.id;
            res[i]["description"] = item.description;
            res[i]["location"] = item.location;
            res[i]["date"] = item.date;
            res[i]["isLost"] = item.isLost;
            i++;
        }
        return res;
    });

    // ---------- SEARCH BY ID ----------
    CROW_ROUTE(app, "/search/<int>")
    ([](int id){
        auto results = tracker.searchById(id);
        crow::json::wvalue res;

        int i = 0;
        for (auto& item : results) {
            res["results"][i]["id"] = item.id;
            res["results"][i]["description"] = item.description;
            res["results"][i]["location"] = item.location;
            res["results"][i]["date"] = item.date;
            res["results"][i]["isLost"] = item.isLost;
            i++;
        }

        return res;
    });

    // ---------- PORT (Railway-safe) ----------
    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::stoi(portEnv) : 8080;

    std::cout << "[INFO] Server running on port " << port << std::endl;
    app.port(port).multithreaded().run();
}
