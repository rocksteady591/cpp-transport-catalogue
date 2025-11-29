#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "transport_catalogue.h"
#include "input_reader.h"

class StatReader {
public:
    StatReader() = default;

    void ParseAndPrintStat(std::istream& in, std::ostream& out, transport::TransportCatalogue& tc);

private:
    parce::InputReader input_reader_;
};
