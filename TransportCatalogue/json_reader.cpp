#include "json_reader.h"
#include "json.h"
#include "request_handler.h"
#include <iostream>

void JsonReader::input_json_reader(transport::TransportCatalogue& tc) {
    json::Document doc = json::Load(std::cin);
    const json::Node& root = doc.GetRoot();

    RequestHandler handler;
    handler.Serialization(tc, root);

    //json::Node result = handler.DeSerialization(tc, root);
    //json::Print(json::Document(result), std::cout);
}
