#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "transport_catalogue.h"
#include "input_reader.h"

class StatReader {
public:
    StatReader() = default;

    void ParseAndPrintStat(TransportCatalogue& tc);

private:
    InputReader input_reader_;
};
