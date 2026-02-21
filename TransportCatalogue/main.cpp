#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"
#include "transport_router.h"
#include <sstream>
#include <iostream>


int main() {
    transport::TransportCatalogue tc;
    std::istream& in = std::cin;
    json::Document doc = json::Load(in);
    const json::Node& root = doc.GetRoot();

    JsonReader json_reader;
    json_reader.ReadAndExecuteBaseRequests(tc, root);

    TransportRouter router(tc);

    json::Node result = json_reader.ExecuteStatRequests(tc, root, router);
    std::ostringstream out;
    json::Print(json::Document(result), out);
    std::cout << out.str() << "\n";
    return 0;
}
