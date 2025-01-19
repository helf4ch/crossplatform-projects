#include "libmyhttp/myhttp.hpp"
#include "libmyserial/myserial.hpp"
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#define READ_TIME_MIN 15

std::string get_ctime_string() {
  std::stringstream buffer;
  std::time_t t = std::time(0);
  std::tm *now = std::localtime(&t);
  buffer << std::put_time(now, "%d.%m.%Y %H:%M:%S");
  return buffer.str();
}

void write_to_db(sqlite3 *db, const nlohmann::json &data) {
  std::stringstream ss;
  ss << "INSERT INTO records (temperature, pressure, humidity, "
        "wind_speed, feels_like) VALUES (";
  ss << data["temperature"] << ", ";
  ss << data["pressure"] << ", ";
  ss << data["humidity"] << ", ";
  ss << data["wind_speed"] << ", ";
  ss << data["feels_like"] << ");";

  std::string insert_sql = ss.str();

  int insertion_res = sqlite3_exec(db,insert_sql.c_str(), nullptr, nullptr, nullptr);
  if (insertion_res != SQLITE_OK) {
    // error
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Usage: [port]\n";
    return 0;
  }

  sqlite3 *db;
  sqlite3_stmt *stmt;

  sqlite3_open_v2("records.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                  nullptr);

  if (db == NULL) {
    printf("Failed to open DB\n");
    return 1;
  }

  std::string create_table_sql = R"(
    CREATE TABLE IF NOT EXISTS records (
	    unixtime INTEGER PRIMARY KEY DEFAULT (strftime('%s', 'now')),
	    temperature REAL DEFAULT 0,
	    pressure INTEGER DEFAULT 0,
	    humidity INTEGER DEFAULT 0,
	    wind_speed REAL DEFAULT 0,
	    feels_like REAL DEFAULT 0
	  ) WITHOUT rowid;
	)";

  int table_creation_res =
      sqlite3_exec(db, create_table_sql.c_str(), nullptr, nullptr, nullptr);
  if (table_creation_res != SQLITE_OK) {
    // error
  }

  std::string com = argv[1];

  my::Serial port(com);

  port.set_baudrate(my::Serial::BaudRate::BR_115200);
  port.set_parity(my::Serial::Parity::COM_PARITY_EVEN);
  port.set_bytesize(my::Serial::ByteSize::SIZE_8);
  port.set_stopbit(my::Serial::StopBits::STOPBIT_ONE);
  port.set_timeout(0);

  port.setup();

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(READ_TIME_MIN * 60));

    std::cout << "===============\n";

    std::cout << "Wakeup at " << get_ctime_string() << "\n\n";

    std::string buf;
    int res = port.read(buf);
    if (res > 0) {
      if (buf[res - 1] != '\n') {
        std::cout << "read failed\n";
        port.flush();
      } else {
        std::cout << "Readed from port:\n";
        std::cout << buf;

        auto data = nlohmann::json::parse(buf);
        write_to_db(db, data);
      }
    } else {
      std::cout << "Got nothing\n";
    }

    std::cout << "===============\n\n";
  }

  // my::http::Server server({"127.0.0.1", 8080});
  // server.handle();

  sqlite3_close(db);

  return 0;
}
