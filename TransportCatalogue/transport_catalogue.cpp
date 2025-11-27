#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>

size_t CombineHash(size_t current_hash, size_t next_component_hash) {
	return current_hash * P + next_component_hash;
}

size_t StopHasher::operator()(const Stop& stop) const {
	size_t hash = 0;
	hash = CombineHash(hash, string_hasher(stop.name_));
	hash = CombineHash(hash, int_hasher(stop.latitude_));
	hash = CombineHash(hash, int_hasher(stop.longitude_));
	return hash;
}

size_t BusHasher::operator()(const Bus& bus) const {
	size_t hash = 0;
	hash = CombineHash(hash, string_hasher(bus.number_));
	for (size_t i = 0; i < bus.route_.size(); ++i) {
		hash = CombineHash(hash, string_hasher(bus.route_[i]));
	}
	return hash;
}

void TransportCatalogue::AddStop(std::string&& stop_name, double latitude, double longitude) {
	Stop stop;
	stop.name_ = std::move(stop_name);
	stop.latitude_ = latitude;
	stop.longitude_ = longitude;
	stops_.insert(std::move(stop));
}
void TransportCatalogue::AddBus(std::string&& number, std::vector<std::string>&& route) {
	Bus bus;
	bus.number_ = std::move(number);
	bus.route_ = std::move(route);
	buss_.insert(std::move(bus));
}

std::string TransportCatalogue::Substring(const std::string_view stop_name) {
	std::string word;
	size_t start = stop_name.find_first_not_of(" ");
	size_t end = stop_name.find_last_not_of(" ");
	word = stop_name.substr(start, end - start + 1);
	return word;
}

std::optional<Stop> TransportCatalogue::SearchStop(const std::string_view stop_name) {
	std::string word = Substring(stop_name);
	for (const Stop&  stop: stops_) {
		if (stop.name_ == word) {
			return stop;
		}
	}
	return std::nullopt;
}

std::optional<std::vector<std::string>> TransportCatalogue::SearchBus(const std::string_view bus_name) {
	for (const Bus& bus : buss_) {
		if (bus.number_ == bus_name) {
			return bus.route_;
		}
	}
	return std::nullopt;
}

size_t TransportCatalogue::GetUniqueStops(std::vector<std::string> stops) {
	std::sort(stops.begin(), stops.end());
	auto it = std::unique(stops.begin(), stops.end());
	stops.erase(it, stops.end());
	return stops.size();
}


double TransportCatalogue::CalculateFullDistance(std::vector<std::string>& stops_names) {
	if (stops_names.empty()) {
		return 0.0;
	}

	double one_way_distance = 0.0;

	// Шаг 1: Считаем расстояние в одну сторону для всех сегментов
	// Используем stops_names[i] и stops_names[i + 1]
	for (size_t i = 0; i < stops_names.size() - 1; ++i) {

		// 1. Поиск остановок
		std::optional<Stop> stop1 = SearchStop(stops_names[i]);
		std::optional<Stop> stop2 = SearchStop(stops_names[i + 1]);

		// 2. Проверка наличия и расчет
		if (stop1.has_value() && stop2.has_value()) {

			// Инициализация Coordinates (без префикса geo::)
			Coordinates cd1{ stop1.value().latitude_, stop1.value().longitude_ };
			Coordinates cd2{ stop2.value().latitude_, stop2.value().longitude_ };

			// Вызов ComputeDistance (без префикса geo::)
			one_way_distance += ComputeDistance(cd1, cd2);
		}
	}

	// Шаг 2: Применяем логику ТЗ (умножение на 2.0 только для некольцевых маршрутов)
	double final_distance = one_way_distance;

	// Некольцевой маршрут: первая остановка != последней (едем туда и обратно)
	if (stops_names[0] != stops_names.back()) {
		// ТЗ: A-B-C -> Dist(AB) + Dist(BC) + Dist(CB) + Dist(BA) = 2 * (Dist(AB) + Dist(BC))
		final_distance *= 2.0;
	}
	// Кольцевой маршрут: первая == последней (полная длина уже посчитана)

	return final_distance;
}

size_t TransportCatalogue::GetStopsCount() const {
	return stops_.size();
}
size_t TransportCatalogue::GetBussCount() const {
	return buss_.size();
}