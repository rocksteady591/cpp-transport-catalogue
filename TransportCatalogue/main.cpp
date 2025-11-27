#include "stat_reader.h"
#include "transport_catalogue.h"

int main()
{
    TransportCatalogue tc;
    StatReader().ParseAndPrintStat(tc);
    return 0;
}

