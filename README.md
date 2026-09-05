# clithr

A simple weather CLI tool made in C++ with a cache.

## Requirements

* g++ (C++17 support, GCC 13+ recommended)
* An [OpenWeatherMap](https://openweathermap.org/api) API key

## Installation

### Debian/Ubuntu

Install dependencies:
```bash
sudo apt install libcurl4-openssl-dev nlohmann-json3-dev
```

Create a `.env` file in the project root with your API key:
```
OWM_API_KEY=your_api_key_here
```

Compile:
```bash
g++ -std=c++17 main.cpp -o clithr -lcurl -Wall
```

## Usage

```bash
./clithr <city>
```

Example:
```bash
./clithr paris
```

Results are cached for 60 seconds to avoid hitting the API repeatedly for the same city.
