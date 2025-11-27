#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_set>

const size_t P = 37;

struct Stop {
	Stop() = default;
	Stop(const Stop&) = default;
	Stop(Stop&&) = default;
	Stop& operator=(const Stop& stop) {
		name_ = stop.name_;
		latitude_ = stop.latitude_;
		longitude_ = stop.longitude_;
		return *this;
	}
	Stop& operator=(Stop&&) = default;
	bool operator==(const Stop& other) const {
		return name_ == other.name_ && latitude_ == other.latitude_ && longitude_ == other.longitude_;
	}

	std::string name_;
	double latitude_;
	double longitude_;
};

struct Bus {
	Bus() = default;
	Bus(const Bus&) = default;
	Bus(Bus&&) = default;
	Bus& operator=(const Bus& bus) {
		number_ = bus.number_;
		route_ = bus.route_;
		return *this;
	}
	Bus& operator=(Bus&&) = default;
	bool operator==(const Bus& other) const {
		return number_ == other.number_ && route_ == other.route_;
	}

	std::string number_;
	std::vector<std::string> route_;
};

struct StopHasher {
public:
	size_t operator()(const Stop& stop) const;
private:
	std::hash<std::string> string_hasher;
	std::hash<double> int_hasher;
};

struct BusHasher {
public:
	size_t operator()(const Bus& bus) const;
private:
	std::hash<std::string> string_hasher;
};

class TransportCatalogue {
public:
	void AddStop(std::string&& stop_name, double lotitude, double longitude);
	void AddBus(std::string&& number, std::vector<std::string>&& route);
	std::optional<Stop> SearchStop(const std::string_view stop_name);
	std::optional<std::vector<std::string>> SearchBus(const std::string_view bus_name);
	std::string Substring(const std::string_view stop_name);
	double CalculateFullDistance(std::vector<std::string>& stops_names);
	size_t GetUniqueStops(std::vector<std::string> stops);
	size_t GetStopsCount() const;
	size_t GetBussCount() const;
private:
	std::unordered_set<Stop, StopHasher> stops_;
	std::unordered_set<Bus, BusHasher> buss_;
};

size_t CombineHash(size_t current_hash, size_t nex_component_hash);