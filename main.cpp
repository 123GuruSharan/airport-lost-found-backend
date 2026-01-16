#define CROW_DISABLE_BOOST
#include "crow_all.h"
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <cstdlib>

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
            if (item.id == id) results.push_back(item);
        }
        return results;
    }
};

HashTable tracker;

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
    return "Backend is running!";
});

    // Add Item API
    CROW_ROUTE(app, "/add")
    ([&](const crow::request& req){
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400);

        tracker.insertItem(Item(
            x["id"].i(),
            x["description"].s(),
            x["location"].s(),
            x["date"].s(),
            x["isLost"].b()
        ));

        return crow::response("Item added successfully");
    });

    // Get All Items
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
            result[i]["type"] = item.isLost ? "Lost" : "Found";
            i++;
        }
        return result;
    });
    // Search by ID
    CROW_ROUTE(app, "/search/<int>")
    ([](int id){
        auto results = tracker.searchById(id);
        crow::json::wvalue result;

        int i = 0;
        for (auto& item : results) {
            result[i]["id"] = item.id;
            result[i]["description"] = item.description;
            result[i]["location"] = item.location;
            result[i]["date"] = item.date;
            result[i]["type"] = item.isLost ? "Lost" : "Found";
            i++;
        }
        return result;
    });
    

    // ✅ Render compatible PORT handling
    const char* portEnv = getenv("PORT");
    int port = portEnv ? stoi(portEnv) : 8080;

int port = 8080;
if (const char* p = std::getenv("PORT")) {
    port = std::stoi(p);
}

app.port(port).multithreaded().run();
}
