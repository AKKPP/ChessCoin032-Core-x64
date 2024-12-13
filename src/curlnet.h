#ifndef CURLNET_H
#define CURLNET_H

#include <QCoreApplication>
#include <curl/curl.h>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

bool isHttps(const QString& url)
{
    // Regular expression to extract the scheme from the URL
    QRegularExpression schemeRegex("(https?)://");

    // Search for the scheme in the URL
    QRegularExpressionMatch match = schemeRegex.match(url);
    if (match.hasMatch()) {
        // Check if the scheme is "https"AF
        return match.captured(1) == "https";
    }

    // If no scheme is found, default to HTTP
    return false;
}

QString getCertFilePath()
{
    QString certPath = QCoreApplication::applicationDirPath();
    certPath.append("/certs/cacert.pem");
    return certPath;
}

QString sendWebRequest(const QString& url)
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer = "";
    QString ret = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        if (isHttps(url))
            curl_easy_setopt(curl, CURLOPT_CAINFO, getCertFilePath().toStdString().c_str());

        // Set the custom header
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            readBuffer = curl_easy_strerror(res);
        }

        ret = QString::fromStdString(readBuffer);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return ret;
}

#endif // CURLNET_H
