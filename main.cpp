#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <ctime>

using json = nlohmann::json;
using namespace std;

string url = "https://api.openweathermap.org/data/2.5/weather?q=";

const string dbPath = "db.txt";
const int CACHE_DUREE = 60;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = nmemb * size;
    string* response = (string*)userdata;
    response->append(ptr, total_size);
    return total_size;
}
std::unordered_map<string, string> loadEnv(const string& path) {
    std::unordered_map<string, string> env;
    std::ifstream file(path);
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        string key = line.substr(0, eq);
        string value = line.substr(eq + 1);
        env[key] = value;
    }
    return env;
}

auto env = loadEnv(".env");
string apiKey = env["OWM_API_KEY"];
string url2 = "&units=metric&appid=" + apiKey;

int main(int argc, char* argv[]) {
    string city = (argc < 2) ? "paris" : argv[1];
    std::time_t nowtime = std::time(nullptr);

    
    if (env.find("OWM_API_KEY") == env.end()) {
    std::cerr << "Error: OWM_API_KEY missin in .env\n";
    return 1;
}

    vector<string> keptLines;
    bool cityTrue = false;
    {
        ifstream db(dbPath);
        string line;
        while (getline(db, line)) {
            istringstream iss(line);
            string cityInLine;
            long long storedTimestamp = 0;
            iss >> cityInLine >> storedTimestamp;

            if (cityInLine == city) {
                long long age = nowtime - storedTimestamp;
                if (age < CACHE_DUREE) {
                    cityTrue = true;
                    keptLines.push_back(line); 
                }
                
            } else {
                keptLines.push_back(line);
            }
        }
    }

    {
        ofstream out(dbPath, ios::trunc);
        for (auto& l : keptLines) out << l << "\n";
    }

    if (cityTrue) {
        json data;
        ifstream city_cache(city + ".json", ifstream::binary);
        city_cache >> data;
        cout << "city: " << city << "\n";
        cout << "id: " << data["id"] << endl;
        cout << "main: " << data["weather"][0]["main"] << endl;
        cout << "temp: " << data["main"]["temp"] << "c";
        cout << "  feel : " << data["main"]["feels_like"] << "c" << endl;
        cout << "cache\n";
    } else {
        string fullUrl = url + city + url2;
        string response_data;

        curl_global_init(CURL_GLOBAL_ALL);
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        json data = json::parse(response_data);
        cout << "city: " << city << "\n";
        cout << "id: " << data["id"] << endl;
        cout << "main: " << data["weather"][0]["main"] << endl;
        cout << "temp: " << data["main"]["temp"] << "c";
        cout << "  feel : " << data["main"]["feels_like"] << "c" << endl;
        cout << "web\n";

        ofstream output_file(city + ".json");
        output_file << response_data;

        ofstream db(dbPath, ios::app);
        db << city << " " << nowtime << "\n";
    }

    return 0;
}