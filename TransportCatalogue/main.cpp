#include "json_reader.h"
#include "transport_catalogue.h"
#include <chrono>
#include <iostream>

int main() {
    //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    //setlocale (LC_ALL,"Russian");
    transport::TransportCatalogue tc;
    JsonReader json;
    json.input_json_reader(tc);
    //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //std::chrono::duration res = end - start;
    //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(res).count() << std::endl;
    return 0;
}
