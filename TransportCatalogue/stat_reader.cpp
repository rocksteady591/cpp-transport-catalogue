#include "stat_reader.h"
#include "transport_catalogue.h"
#include "geo.h"

StatReader::StatReader() = default;

void StatReader::ParseAndPrintStat(TransportCatalogue& tc) {
	size_t count_requests = 0;
	std::cin >> count_requests;
	std::cin.ignore();
	std::string line;
	std::vector<std::string> requests;
	requests.reserve(count_requests);
	while (std::getline(std::cin, line) && count_requests != 0) {
		requests.push_back(std::move(line));
		--count_requests;
	}
	InputReader().ParseLine(std::move(requests), tc);
	count_requests = std::stoi(line);
	
	requests.clear();
	while (count_requests != 0 && std::getline(std::cin, line)) {
		requests.push_back(std::move(line));
		--count_requests;
	}



	std::vector<std::pair<std::string, std::optional<std::vector<std::string>>>> result = 
		InputReader().FindAndOutput(std::move(requests), tc);
	for (auto& val : result) {
		if (val.second == std::nullopt) {
			std::cout << val.first << "\n";
		}
		else {
			std::cout << val.first;
			size_t count_stops = val.second.value().size();
			double distance = tc.CalculateFullDistance(val.second.value());
			if (tc.Substring(val.second.value()[0]) == tc.Substring(val.second.value()[count_stops - 1])) {
				std::cout << count_stops << " stops on route, " << tc.GetUniqueStops(val.second.value()) << " unique stops, " 
					<< distance << " route length" << "\n";
			}
			else {
				std::cout << count_stops * 2 - 1 << " stops on route, " << tc.GetUniqueStops(val.second.value()) << " unique stops, "
					<< distance << " route length" << "\n";
			}
		}
	}
}