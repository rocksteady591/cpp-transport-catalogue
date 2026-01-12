#include "request_handler.h"
#include "transport_catalogue.h"
#include <sstream>
#include <iostream>


int main() {
    //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    transport::TransportCatalogue tc;
    RequestHandler request_handler;
    std::istream& in = std::cin;
    std::ostringstream result_out = request_handler.ReadJson(tc, in);
    //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //std::chrono::duration res = end - start;
    //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(res).count() << std::endl;
    std::cout << result_out.str() << "\n";
    return 0;
}
