#include "xml.h"
#include "json.h"
#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

struct Spending {
    string category;
    int amount;
};

bool operator==(const Spending& lhs, const Spending& rhs) {
    return lhs.category == rhs.category && lhs.amount == rhs.amount;
}

ostream& operator<<(ostream& os, const Spending& s) {
    return os << '(' << s.category << ": " << s.amount << ')';
}

int CalculateTotalSpendings(const vector<Spending>& spendings) {
    int result = 0;
    for (const Spending& s: spendings) {
        result += s.amount;
    }
    return result;
}

string MostExpensiveCategory(const vector<Spending>& spendings) {
    auto compare_by_amount =
            [](const Spending& lhs, const Spending& rhs) {
                return lhs.amount < rhs.amount;
            };
    return max_element(begin(spendings), end(spendings),
                       compare_by_amount)->category;
}

vector<Spending> LoadFromXml(istream& input) {
    vector<Spending> result;

    Xml::Document doc = Xml::Load(input);

    for (const Xml::Node& child : doc.GetRoot().Children()) {
        result.push_back({
            child.AttributeValue<string>("category"),
            child.AttributeValue<int>("amount")
        });
    }

    return result;
}

vector<Spending> LoadFromJson(istream& input) {
    vector<Spending> result;

    Json::Document doc = Json::Load(input);

    for (const Json::Node& child : doc.GetRoot().AsArray()) {
        result.push_back({
            child.AsMap().at("category").AsString(),
            child.AsMap().at("amount").AsInt()
        });
    }

    return result;
}

void TestLoadFromXml() {
    istringstream xml_input(R"(<july>
    <spend amount="2500" category="food"></spend>
    <spend amount="1150" category="transport"></spend>
    <spend amount="5780" category="restaurants"></spend>
    <spend amount="7500" category="clothes"></spend>
    <spend amount="23740" category="travel"></spend>
    <spend amount="12000" category="sport"></spend>
  </july>)");

    const vector<Spending> spendings = LoadFromXml(xml_input);

    const vector<Spending> expected = {
            {"food",        2500},
            {"transport",   1150},
            {"restaurants", 5780},
            {"clothes",     7500},
            {"travel",      23740},
            {"sport",       12000}
    };
    ASSERT_EQUAL(spendings, expected);
}

void TestXmlLibrary() {
    istringstream xml_input(R"(<july>
    <spend amount="2500" category="food"></spend>
    <spend amount="23740" category="travel"></spend>
    <spend amount="12000" category="sport"></spend>
  </july>)");

    Xml::Document doc = Xml::Load(xml_input);
    const Xml::Node& root = doc.GetRoot();
    ASSERT_EQUAL(root.Name(), "july");
    ASSERT_EQUAL(root.Children().size(), 3u);

    const Xml::Node& food = root.Children().front();
    ASSERT_EQUAL(food.AttributeValue<string>("category"), "food");
    ASSERT_EQUAL(food.AttributeValue<int>("amount"), 2500);

    const Xml::Node& sport = root.Children().back();
    ASSERT_EQUAL(sport.AttributeValue<string>("category"), "sport");
    ASSERT_EQUAL(sport.AttributeValue<int>("amount"), 12000);

    Xml::Node july("july", {});
    Xml::Node transport("spend", {{"category", "transport"},
                             {"amount",   "1150"}});
    july.AddChild(transport);
    ASSERT_EQUAL(july.Children().size(), 1u);
}

void TestLoadFromJson() {
    istringstream json_input(R"([
    {"amount": 2500, "category": "food"},
    {"amount": 1150, "category": "transport"},
    {"amount": 5780, "category": "restaurants"},
    {"amount": 7500, "category": "clothes"},
    {"amount": 23740, "category": "travel"},
    {"amount": 12000, "category": "sport"}
  ])");

    const vector<Spending> spendings = LoadFromJson(json_input);

    const vector<Spending> expected = {
            {"food", 2500},
            {"transport", 1150},
            {"restaurants", 5780},
            {"clothes", 7500},
            {"travel", 23740},
            {"sport", 12000}
    };
    ASSERT_EQUAL(spendings, expected);
}

void TestJsonLibrary() {
    istringstream json_input(R"([
    {"amount": 2500, "category": "food"},
    {"amount": 1150, "category": "transport"},
    {"amount": 12000, "category": "sport"}
  ])");

    Json::Document doc = Json::Load(json_input);
    const vector<Json::Node>& root = doc.GetRoot().AsArray();
    ASSERT_EQUAL(root.size(), 3u);

    const map<string, Json::Node>& food = root.front().AsMap();
    ASSERT_EQUAL(food.at("category").AsString(), "food");
    ASSERT_EQUAL(food.at("amount").AsInt(), 2500);

    const map<string, Json::Node>& sport = root.back().AsMap();
    ASSERT_EQUAL(sport.at("category").AsString(), "sport");
    ASSERT_EQUAL(sport.at("amount").AsInt(), 12000);

    Json::Node transport(map<string, Json::Node>{{"category", Json::Node("transport")}, {"amount", Json::Node(1150)}});
    Json::Node array_node(vector<Json::Node>{transport});
    ASSERT_EQUAL(array_node.AsArray().size(), 1u);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestXmlLibrary);
    RUN_TEST(tr, TestLoadFromXml);
    RUN_TEST(tr, TestJsonLibrary);
    RUN_TEST(tr, TestLoadFromJson);
}