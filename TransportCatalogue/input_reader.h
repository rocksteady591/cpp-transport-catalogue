#pragma once
#include<vector>
#include<string>
#include "transport_catalogue.h"

class InputReader {
public:
	InputReader();
	void ParseLine(std::vector<std::string>&& requests, TransportCatalogue& tc);
	std::vector<std::pair<std::string, std::optional<std::vector<std::string>>>> FindAndOutput(std::vector<std::string>&& requests, TransportCatalogue& tc);
};