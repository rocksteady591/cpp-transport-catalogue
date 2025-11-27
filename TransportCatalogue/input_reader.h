#pragma once
#include <vector>
#include <string>
#include "transport_catalogue.h"

class InputReader {
public:
    InputReader() = default;

    // Обработка запросов Stop и Bus для заполнения справочника
    void ParseLines(const std::vector<std::string>& lines, TransportCatalogue& tc);

private:
    std::string Trim(const std::string& str) const;
};
