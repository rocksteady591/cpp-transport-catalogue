#include "input_reader.h"
#include "transport_catalogue.h"
#include <sstream>

InputReader::InputReader() = default;


void InputReader::ParseLine(std::vector<std::string>&& requests, TransportCatalogue& tc) {
	for (std::string& line : requests) {
		size_t start = line.find_first_not_of(" ");
		size_t end = line.find_first_of(" ", start);
		std::string comand = line.substr(start, end - start);
		std::string stop_or_bus_name;
		if (comand == "Stop") {
			start = end + 1;
			end = line.find_first_of(":");
			stop_or_bus_name = line.substr(start, end - start);
			start = end + 1;
			end = line.find_first_of(",");
			double latitude = std::stof(line.substr(start, end - start));
			start = end + 1;
			end = line.find_last_not_of(" ");
			double longitude = std::stof(line.substr(start, end - start));
			tc.AddStop(std::move(stop_or_bus_name), latitude, longitude);
		}
		else if(comand == "Bus") {
			char separator = '>';
			std::vector<std::string> stops;
			start = end + 1;
			end = line.find_first_of(":");
			stop_or_bus_name = line.substr(start, end - start);
			start = end + 1;
			end = line.find_first_of(separator);
			if (end == std::string::npos) {
				separator = '-';
				end = line.find_first_of(separator);
			}
			std::string stops_line = line.substr(start);
			std::istringstream iss(stops_line);
			std::string token;
			while (std::getline(iss, token, separator)) {
				stops.push_back(std::move(tc.Substring(token)));
			}
			tc.AddBus(std::move(stop_or_bus_name), std::move(stops));
		}
	}
}

std::vector<std::pair<std::string, std::optional<std::vector<std::string>>>> 
InputReader::FindAndOutput(std::vector<std::string>&& requests, TransportCatalogue& tc) {
	using namespace std::literals;
	std::vector<std::pair<std::string, std::optional<std::vector<std::string>>>> result;
	for (std::string& line : requests) {
		size_t start = line.find_first_not_of(" ");
		size_t end = line.find_first_of(" ", start);
		std::string comand = line.substr(start, end - start);
		std::string stop_or_bus_name;
		if (comand == "Bus") {
			start = end + 1;
			end = line.find_last_not_of(" ");
			stop_or_bus_name = line.substr(start, end - start + 1);
			std::optional<std::vector<std::string>> route = tc.SearchBus(stop_or_bus_name);
			if (route.has_value()) {
				result.push_back({ "Bus "s + stop_or_bus_name + ": "s, route});
			}
			else {
				result.push_back({ "Bus "s + stop_or_bus_name + ": not found"s, route });
			}
		}
		
	}
	return result;
}