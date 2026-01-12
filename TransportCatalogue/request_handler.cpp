#include "request_handler.h"
#include "json.h"
#include "json_reader.h"
#include <istream>

using namespace std;


std::ostringstream RequestHandler::ReadJson(transport::TransportCatalogue& tc, std::istream& in) {
    json::Document doc = json::Load(in);
    const json::Node& root = doc.GetRoot();

    JsonReader json_reader;
    json_reader.DeSerialization(tc, root);

    json::Node result = json_reader.Serialization(tc, root);
    std::ostringstream out;
    json::Print(json::Document(result), out);
    return out;
}


