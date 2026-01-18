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

    // ---------- HEALTH CHECK ----------
    CROW_ROUTE(app, "/")([](){
        return "Backend is running!";
    });

    // ---------- ADD ITEM (POST + OPTIONS) ----------
    CROW_ROUTE(app, "/add")
        .methods(crow::HTTPMethod::Post, crow::HTTPMethod::Options)
    ([](const crow::request& req){
        crow::response res;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");

        // Handle CORS preflight
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            return res;
        }

        auto x = crow::json::load(req.body);
        if (!x) {
            res.code = 400;
            res.body = "Invalid JSON";
            return res;
        }

        if (!x.has("id") || !x.has("description") || !x.has("location")
            || !x.has("date") || !x.has("isLost")) {
            res.code = 400;
            res.body = "Missing required fields";
            return res;
        }

        tracker.insertItem(Item(
            x["id"].i(),
            x["description"].s(),
            x["location"].s(),
            x["date"].s(),
            x["isLost"].b()
        ));

        res.code = 200;
        res.body = "Item added successfully";
        return res;
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
    int port = getenv("PORT") ? stoi(getenv("PORT")) : 8080;
    cout << "[INFO] Server running on port " << port << endl;

    app.port(port).multithreaded().run();
}
