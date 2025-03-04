
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "threadPool.hpp"

struct wsData{
    float sum = 0.0;
    int count = 0;
    float max = -100.0;
    float min = +100.0;
};

std::mutex dataMapMutex;

void process(std::string str, std::map<std::string, wsData>& dataMap) {
    std::map<std::string, wsData> localDataMap;
    std::stringstream buffer(str); 
    std::string line;
    while (getline(buffer, line)) {
        std::string delimiter = ";";
        std::string token = line.substr(0, line.find(delimiter));
        float temp = std::stof(line.substr(line.find(delimiter) + 1, line.length()));

        auto& data = localDataMap[token];
        data.count++;
        data.sum += temp;
        if (temp > data.max) {
            data.max = temp;
        }
        if (temp < data.min) {
            data.min = temp;
        }
    }

    std::lock_guard<std::mutex> lock(dataMapMutex);
    for (const auto& [key, localData] : localDataMap) {
        auto& globalData = dataMap[key];
        globalData.sum += localData.sum;
        globalData.count += localData.count;
        globalData.max = std::max(globalData.max, localData.max);
        globalData.min = std::min(globalData.min, localData.min);
    }
}

int main(int argc, char* argv[])
{

    const char* file = "measurements.txt";
    // Structure to save data
    std::map<std::string, wsData> dataMap;
    if (argc > 1){
        // ... or the first argument.
        file = argv[1];
    }
    std::ifstream fh(file);
    if (not fh.is_open()){
        std::cerr << "Unable to open '" << file << "'" << std::endl;
        return 1;
    }
    
    std::string ss;
    std::string line;
    int line_counter = 0;
    size_t n_threads = 16;
    ThreadPool threadPool(n_threads);

    while (std::getline(fh, line)) {
        ss += line + '\n';
        line_counter++;
        if (line_counter > 1000){
            line_counter = 0;
            threadPool.enqueue(process, std::move(ss), std::ref(dataMap));
            ss = "";
        }
    }

    if (!ss.empty()) {
        threadPool.enqueue(process, std::move(ss), std::ref(dataMap));
    }

    threadPool.join();
    
    for(auto v : dataMap){
        std::string token = v.first;
        wsData data = v.second;
        std::cout << token << ": " << "avg:" << data.sum/data.count << " min:" << data.min << " max:" << data.max << " count:" << data.count << std::endl;
    }


    // Always close the file when done
    fh.close();

    return 0;
}
