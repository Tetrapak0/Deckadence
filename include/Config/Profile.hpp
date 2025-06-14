#pragma once

#include <string>
#include <vector>

#include "Item.hpp"

using std::vector;
using std::string;

class Profile {
    friend class Client;
public:
    string name;
    vector<Item> items;

    int rows = 4;
    int columns = 6;

    Profile() = default;
    Profile(string name, const int rows, const int columns) : name(std::move(name)), rows(rows), columns(columns),
                                                              m_name(name), m_rows(rows), m_columns(columns) {
        items.reserve(rows*columns);
        for (int i = 0; i < rows*columns; ++i) {
            items.emplace_back();
        }
    }
private:
    string m_name = name;
    int m_rows = rows;
    int m_columns = columns;
};
