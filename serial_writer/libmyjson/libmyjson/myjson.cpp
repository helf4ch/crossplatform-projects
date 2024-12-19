#include "myjson.hpp"

#include <cstdio>
#include <deque>
#include <sstream>
#include <unordered_map>

#include <iostream>

class my::Json::JsonImpl {
public:
  std::unordered_map<std::string, std::string> parsed;

  void parse(std::stringstream &ss) {
    std::deque<char> stack;
    std::deque<std::string> name_stack;

    std::string name;
    std::string value;
    bool in_str = false;
    bool in_name = true;
    char ch;
    while (ss >> ch) {
      std::cout << ch << '\n';
      switch (ch) {
      case '{': {
        std::cout << "in {\n";
        if (in_name) {
          stack.push_back(ch);
        } else {
          name_stack.push_back(name);
          in_name = true;
        }
        break;
      }
      case '}': {
        std::cout << "in }\n";
        if (stack.back() != '{') {
          // error
          std::cout << "error\n";
        }
        stack.pop_back();
        std::stringstream prepend;
        for (int i = 0; i < name_stack.size(); ++i) {
          prepend << name_stack[i] << '.';
        }
        parsed[prepend.str() + name] = value;
        std::cout << "parsed " << prepend.str() + name << '\n'; 
        name_stack.pop_back();
        in_name = true;
        break;
      }
      case ':': {
        std::cout << "in :\n";
        if (in_str) {
          break;
        }
        in_name = false;
        break;
      }
      case '"': {
        std::cout << "in \"\n";
        if (stack.back() == '"') {
          in_str = false;
          stack.pop_back();
        } else if (!in_str) {
          in_str = true;
          stack.push_back('"');
        } else {
          // error
          std::cout << "error\n";
        }
        break;
      }
      case '\t':
      case '\n':
      case '\r':
      case ' ': {
        std::cout << "in  (space)\n";
        if (!in_str) {
          break;
        }
        if (in_name) {
          name.push_back(ch);
        } else {
          value.push_back(ch);
        }
        break;
      }
      case ',': {
        std::cout << "in ,\n";
          if (in_str) {
            if (in_name) {
              name.push_back(ch);
            } else {
              value.push_back(ch);
            }
          } else if (!in_name) {
            parsed[name] = value;
            in_name = true;
          }
          break;
        }
      default: {
        std::cout << "in (default)\n";
        if (in_str) {
          if (in_name) {
            name.push_back(ch);
          } else {
            value.push_back(ch);
          }
        } else if (!in_name) {
          value.push_back(ch);
        } else {
          // error
          std::cout << "error\n";
        }
        break;
      }
      }
    }
  }
};

my::Json::Json(const std::string &str) {
  json = new my::Json::JsonImpl;

  std::stringstream ss(str);
  json->parse(ss);
}

my::Json::~Json() { delete json; }

const std::string &my::Json::operator[](const std::string &name) {
  return json->parsed.at(name);
}
