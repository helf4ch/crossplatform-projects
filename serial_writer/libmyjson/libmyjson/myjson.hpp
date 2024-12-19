#pragma once

#include <string>

namespace my {

class Json {
public:
  Json(const std::string &str);

  ~Json();

  const std::string &operator[](const std::string &name);

private:
  class JsonImpl;

  JsonImpl *json;
};

}
