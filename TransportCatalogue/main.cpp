#include "json_reader.h"
#include "transport_catalogue.h"

int main() {
    //setlocale (LC_ALL,"Russian");
    transport::TransportCatalogue tc;
    JsonReader json;
    json.input_json_reader(tc);
    
    return 0;
}
