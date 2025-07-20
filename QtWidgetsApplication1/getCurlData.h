#pragma once
#include <string>
#include <map>
#include <qdatetimeedit.h>


class CurrencyFetcher {
public:
    std::map<std::string, double> fetchRates(const std::string& url);
    std::vector<std::pair<QDate, double>> fetchHistoricalRates(const std::string& url);
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};
