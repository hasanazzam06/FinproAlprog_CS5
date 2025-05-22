#include <fstream>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

using namespace std;

class Log {
private:
    string RFID;
    string action;
    string time;

public:
    Log() = default; 
    Log(string rfid, string act, string t)
        : RFID(rfid), action(act), time(t) {}

    Log(string param) {
        stringstream ss(param);
        getline(ss, RFID, ' ');
        getline(ss, action, ' ');
        getline(ss, time, ' ');
    }
    
    friend void to_json(json& j, const Log& l);
    friend void from_json(const json& j, Log& l);
};

void to_json(json& j, const Log& l){
    j = json{{"rfid", l.RFID}, {"action", l.action}, {"time", l.time}};
}

void from_json(const json& j, Log& l) {
    j.at("rfid").get_to(l.RFID);
    j.at("action").get_to(l.action);
    j.at("time").get_to(l.time);
}

int exportToJSON(const vector<Log>& logs, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) return 0;

    json j = logs; // otomatis pakai to_json()
    file << setw(4) << j << endl; // buat tampil rapi
    file.close();
    return 1;
}
int readFromJSON(vector<Log>& logs, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return 0;

    json j;
    file >> j;
    file.close();

    logs = j.get<vector<Log>>(); // otomatis pakai from_json()
    return 1;
}

int main() {
  Log data ={"skaaddaa11dak","IN","11.59"};
  
  vector<Log> logs;
  
  readFromJSON(logs,"data.json");
  
  logs.push_back(data);
  logs.push_back(data);
  
  exportToJSON(logs,"data.json");
  
  return 0;
}