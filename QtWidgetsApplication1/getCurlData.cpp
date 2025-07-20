#include "getCurlData.h"
#include <curl/curl.h>
#include <iostream>
#include <json.hpp>


size_t CurrencyFetcher::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* response = static_cast<std::string*>(userp);
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::map<std::string, double> CurrencyFetcher::fetchRates(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    std::map<std::string, double> ratesMap;

    if (!curl) {
        std::cerr << "Ошибка инициализации curl" << std::endl;
        return ratesMap;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Ошибка curl: " << curl_easy_strerror(res) << std::endl;
    }
    else {
        try {
            nlohmann::json jsonData = nlohmann::json::parse(response);
            nlohmann::json rates = jsonData["rates"];

            for (auto it = rates.begin(); it != rates.end(); ++it) {
                ratesMap[it.key()] = it.value();

            }
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка парсинга JSON: " << e.what() << std::endl;
        }
    }

    curl_easy_cleanup(curl);
    return ratesMap;
}

std::vector<std::pair<QDate, double>> CurrencyFetcher::fetchHistoricalRates(const std::string& url) {
    std::vector<std::pair<QDate, double>> result;
    CURL* curl = curl_easy_init();
    std::string response;

    if (!curl) return result;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CurrencyApp/1.0");

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        std::cerr << "curl hist error: " << curl_easy_strerror(rc) << std::endl;
        curl_easy_cleanup(curl);
        return result;
    }

    try {
        nlohmann::json jsonData = nlohmann::json::parse(response);
        if (!jsonData.contains("rates")) {
            std::cerr << "No 'rates' in JSON.\n";
            curl_easy_cleanup(curl);
            return result;
        }

        const auto& rates = jsonData["rates"];
        for (auto it = rates.begin(); it != rates.end(); ++it) {
            QDate date = QDate::fromString(QString::fromStdString(it.key()), "yyyy-MM-dd");
            if (!date.isValid()) continue;

            const auto& dayObj = it.value();
            if (!dayObj.is_object() || dayObj.empty()) continue;

            auto jt = dayObj.begin();
            if (jt.value().is_number()) {
                double rate = jt.value().get<double>();
                result.emplace_back(date, rate);
            }
        }

        std::sort(result.begin(), result.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
    }
    catch (const std::exception& e) {
        std::cerr << "hist parse error: " << e.what() << std::endl;
    }

    curl_easy_cleanup(curl);
    return result;
}



