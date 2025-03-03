
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

struct wsData{
    float sum = 0.0;
    int count = 0;
    float max = -100.0;
    float min = +100.0;
};

std::map<std::string, wsData> merge_second_dict_to_first(std::map<std::string, wsData>& first,std::map<std::string, wsData>& second){
    for(auto elem: second){
        std::string token = elem.first;
        wsData& data_to_add = elem.second;

        auto it = first.find(token);
        if(it != first.end()){
            wsData& data = it->second;
            data.count++;
            data.sum += data_to_add.sum;
            if(data_to_add.max > data.max){
                data.max = data_to_add.max;
            }
            if(data_to_add.min < data.min){
                data.min = data_to_add.min;
            }
        }else{
            wsData data{data_to_add.sum,data_to_add.count,data_to_add.max,data_to_add.max};
            first[token] = data;
        }

    }
    return first;
}

std::map<std::string, wsData> process(std::stringstream buffer) {

    // Variable to store each line from the file
    std::string line;
    std::map<std::string, wsData> dataMapBuffer;

    // Read each line from the file and print it
    while (getline(buffer, line)) {
        // Process each line as needed
        std::string delimiter = ";";
        std::string token = line.substr(0, line.find(delimiter));
        float temp = std::stof(line.substr(line.find(delimiter)+1,line.length()));
        
        
        auto it = dataMapBuffer.find(token);
        if(it != dataMapBuffer.end()){
            wsData& data = it->second;
            data.count++;
            data.sum += temp;
            if(temp > data.max){
                data.max = temp;
            }
            if(temp < data.min){
                data.min = temp;
            }
        }else{
            wsData data{temp,1,temp,temp};
            dataMapBuffer[token] = data;
        }
    }

    return dataMapBuffer;
}

int main(int argc, char* argv[])
{
    // Use default file ...
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
    
    std::stringstream ss;
    std::string line;
    int line_counter = 0;
    std::vector<std::thread> threads;

    while (std::getline(fh, line)) {
        ss << line << '\n';
        line_counter++;
        if (line_counter > 20000000){
            line_counter = 0;
            threads.emplace_back(std::thread(process,std::move(ss),std::ref(dataMap)));
            ss = std::stringstream();
        }
    }

    if (!ss.str().empty()) {
        threads.emplace_back(std::thread(process, std::move(ss), std::ref(dataMap)));
    }    

    for(auto& t : threads){
        if(t.joinable()){
            t.join();
        }
    }

    for(auto v : dataMap){
        std::string token = v.first;
        wsData data = v.second;
        std::cout << token << ": " << "avg:" << data.sum/data.count << " min:" << data.min << " max:" << data.max << " count:" << data.count << std::endl;
    }


    // Always close the file when done
    fh.close();

    return 0;
}
