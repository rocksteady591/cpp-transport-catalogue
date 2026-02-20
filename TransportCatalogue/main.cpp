#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"
#include "graph.h"
#include "transport_router.h"
#include <sstream>
#include <iostream>


int main() {
    //std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    transport::TransportCatalogue tc;
    std::istream& in = std::cin;
    json::Document doc = json::Load(in);
    const json::Node& root = doc.GetRoot();

    JsonReader json_reader;
    json_reader.ReadAndExecuteBaseRequests(tc, root);

    Graph graph;
    graph.BuildGraph(tc);
    TransportRouter router(graph);

    json::Node result = json_reader.ExecuteStatRequests(tc, root, router);
    std::ostringstream out;
    json::Print(json::Document(result), out);
    //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //std::chrono::duration res = end - start;
    //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(res).count() << std::endl;
    std::cout << out.str() << "\n";
    return 0;
}
