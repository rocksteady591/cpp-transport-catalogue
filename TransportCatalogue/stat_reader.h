#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "input_reader.h"
#include "transport_catalogue.h"

class StatReader {
public:
	StatReader();
	void ParseAndPrintStat(TransportCatalogue& tc);
private:
	InputReader input_reader_;
};
