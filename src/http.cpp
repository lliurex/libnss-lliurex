/*
    This file is part of libnss-lliurex

    Copyright (C) 2022  Enrique Medina Gremaldos <quiqueiii@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "http.hpp"

#include <json.hpp>

#include <curl/curl.h>

#include <exception>
#include <sstream>

using namespace lliurex::nss;

using namespace edupals;
using namespace edupals::variant;

using namespace std;

HttpClient::HttpClient(string url)
{
}

size_t response_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    stringstream* in=static_cast<stringstream*>(userdata);
    
    for (size_t n=0;n<nmemb;n++) {
        in->put(ptr[n]);
    }
    
    return nmemb;
}

Variant HttpClient::request(string what)
{
    Variant ret;
    stringstream input;
    
    CURL* curl = nullptr;
    struct curl_slist* headers = nullptr;
    CURLcode status;
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    curl = curl_easy_init();
    
    if (!curl) {
        throw runtime_error("Failed to initialize curl");
    }
    
    string address = server+"/"+what;
    
    headers = curl_slist_append(headers, "User-Agent: NSS-HTTP");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&input);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,response_cb);
    
    status = curl_easy_perform(curl);

    if (status != 0) {
        throw runtime_error("curl_easy_perform("+std::to_string(status)+")");
    }
    
    // free curl resources
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
    
    try {
        ret = json::load(input);
    }
    catch (std::exception& e) {
        throw runtime_error(e.what());
    }
    
    return ret;
}
