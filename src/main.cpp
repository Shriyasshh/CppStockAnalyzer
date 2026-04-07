#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream> // REQUIRED
#include "dotenv.h"
#include <curl/curl.h>
#include <json/json.h>

using namespace std;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output)
{
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}

int main()
{
    // Load .env
    dotenv::env.load_dotenv("../.env");

    string api_key = dotenv::env["VANTAGE_API"];

    if (api_key.empty())
    {
        cerr << "Error: VANTAGE_API key not found!" << endl;
        return 1;
    }

    cout << "API Key loaded successfully" << endl;
    cout << "Fetching IBM stock data..." << endl;

    string url =
        "https://www.alphavantage.co/query"
        "?function=TIME_SERIES_DAILY"
        "&symbol=IBM"
        "&apikey=" +
        api_key;

    // 'https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=IBM&apikey=demo'

    CURL *curl;
    CURLcode res;
    string response;

    curl = curl_easy_init();

    if (curl){
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK){
            cout << "\n--- Raw Response Received ---\n";
            // cout << response << endl; //Print Raw Response   
            Json::Value root;
            Json::CharReaderBuilder reader;
            string errs;
            stringstream s(response);

            if (Json::parseFromStream(reader, s, &root, &errs)){
                // ---------- META DATA ----------
                cout << "\nMeta Data\n";

                if (root.isMember("Meta Data")){
                    cout << "Symbol: " << root["Meta Data"]["2. Symbol"] << endl;

                    cout << "Last Refreshed: " << root["Meta Data"]["3. Last Refreshed"] << endl;

                    cout << "Time Zone: " << root["Meta Data"]["5. Time Zone"] << endl;
                }

                // ---------- TIME SERIES ----------
                auto series = root["Time Series (Daily)"];

                if (!series.empty()){
                    //This gets only the first element of the JSON object.
                    
                    // auto it = series.begin();  
                    // cout << "\nLatest Stock Data\n";

                    // cout << "Date: "<< it.name()<< endl;

                    // cout << "Open: " << (*it)["1. open"] << endl;

                    // cout << "High: " << (*it)["2. high"] << endl;

                    // cout << "Low: " << (*it)["3. low"] << endl;

                    // cout << "Close: " << (*it)["4. close"] << endl;

                    // cout << "Volume: " << (*it)["5. volume"] << endl;

                    cout << "\nAll Stock Data\n";
                    int i = 0 ;
                    for (auto it = series.begin(); it != series.end(); ++it){
                        cout << "\nStock Data No: " << ++i << endl;
                        cout << "Date: " << it.name() << endl;

                        cout << "Open: " << (*it)["1. open"] << endl;

                        cout << "High: " << (*it)["2. high"] << endl;

                        cout << "Low: " << (*it)["3. low"] << endl;

                        cout << "Close: " << (*it)["4. close"] << endl;

                        cout << "Volume: " << (*it)["5. volume"] << endl;
                    }
                }
                else{
                    cout << "No stock data found.\n";
                }
            }
            else{
                cout << "JSON parse error\n";
                cout << errs << endl;
            }
        }
        else{
            cout << "Request failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_cleanup(curl);
    }

    return 0;
}