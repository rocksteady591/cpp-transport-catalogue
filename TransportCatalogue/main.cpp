#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
    transport::TransportCatalogue tc;
    StatReader sr;
    sr.ParseAndPrintStat(std::cin, std::cout, tc);
    return 0;
}
