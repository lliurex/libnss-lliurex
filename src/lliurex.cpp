/*
    This file is part of libnss-lliurex

    Copyright (C) 2021  Enrique Medina Gremaldos <quiqueiii@gmail.com>

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

#include <n4d.hpp>

#include <nss.h>
#include <systemd/sd-journal.h>

#include <grp.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <mutex>
#include <chrono>

using namespace lliurex::nss;
using namespace edupals;
using namespace std;

extern "C" enum nss_status _nss_lliurex_setgrent(void);
extern "C" enum nss_status _nss_lliurex_endgrent(void);
extern "C" enum nss_status _nss_lliurex_getgrent_r(struct group* result, char* buffer, size_t buflen, int* errnop);
extern "C" enum nss_status _nss_lliurex_getgrgid_r(gid_t gid, struct group* result, char* buffer, size_t buflen, int* errnop);
extern "C" enum nss_status _nss_lliurex_getgrnam_r(const char* name, struct group* result, char *buffer, size_t buflen, int* errnop);

namespace lliurex
{
    struct Group
    {
        std::string name;
        uint64_t gid;
        std::vector<std::string> members;
    };
    
    std::mutex mtx;
    std::vector<lliurex::Group> groups;
    int index = -1;
    std::chrono::time_point<std::chrono::steady_clock> timestamp;
}

static int push_string(string in,char** buffer, size_t* remain)
{
    size_t fsize = in.size() + 1;
    if (fsize > *remain) {
        return -1;
    }
    
    std::memcpy(*buffer,in.c_str(),fsize);
    
    *remain -= fsize;
    *buffer += fsize;
    
    return 0;
}

static int push_group(lliurex::Group& source, struct group* result, char* buffer, size_t buflen)
{
    char* ptr = buffer;
    
    result->gr_gid = source.gid;
    
    result->gr_name = ptr;
    result->gr_passwd = 0;
    
    if (push_string(source.name,&ptr,&buflen) == -1) {
        return -1;
    }
    
    vector<char*> tmp;
    
    for (string member : source.members) {
        char* q = ptr;
        if (push_string(member,&ptr,&buflen) == -1) {
            return -1;
        }
        tmp.push_back(q);
    }
    
    if ( (sizeof(char*)*(tmp.size()+1)) > buflen) {
        return -1;
    }
    
    result->gr_mem = (char**) ptr;
    
    int n = 0;
    
    for (n = 0;n<tmp.size();n++) {
        result->gr_mem[n] = tmp[n];
    }
    
    result->gr_mem[n] = 0;
    
    return 0;
}

int update_db()
{
    std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    
    n4d::Client client;
    
    double delta = std::chrono::duration_cast<std::chrono::seconds>(now - lliurex::timestamp).count();
    
    if (delta < 2.0) {
        sd_journal_print(LOG_INFO,"cached result: %f seconds",delta);
        return 0;
    }
    
    lliurex::timestamp = now;
    
    try {
        variant::Variant ret = client.call("CDC","getgrall");
        
        lliurex::groups.clear();
        
        for (string key : ret.keys()) {
            lliurex::Group grp;
            
            grp.name = key;
            grp.gid = (uint64_t)ret[key][0].to_int64();
            sd_journal_print(LOG_INFO,"group:(%ld) %s",grp.gid,key.c_str());
            
            for (size_t n=0;n<ret[key][1].count();n++) {
                grp.members.push_back(ret[key][1][n].get_string());
            }
            
            lliurex::groups.push_back(grp);
        }
    }
    catch (std::exception& e) {
        sd_journal_print(LOG_ERR,"update_db: %s",e.what());
        return -1;
    }
    
    return 0;
}

/*
 * Open database
*/
enum nss_status _nss_lliurex_setgrent(void)
{
    std::lock_guard<std::mutex> lock(lliurex::mtx);
    sd_journal_print(LOG_INFO,"lliurex_setgrent()");
    
    lliurex::index = -1;
    
    int db_status = update_db();
    if (db_status==-1) {
        return NSS_STATUS_UNAVAIL;
    }
    
    lliurex::index = 0;
    sd_journal_print(LOG_INFO,"lliurex_setgrent successful");
    return NSS_STATUS_SUCCESS;
}

/*
 * Close database
*/
enum nss_status _nss_lliurex_endgrent()
{
    sd_journal_print(LOG_INFO,"lliurex_endgrent()");
    return NSS_STATUS_SUCCESS;
}

/*
 * Read group entry
*/
enum nss_status _nss_lliurex_getgrent_r(struct group* result, char* buffer, size_t buflen, int* errnop)
{
    std::lock_guard<std::mutex> lock(lliurex::mtx);
    sd_journal_print(LOG_INFO,"lliurex_getgrent %d",lliurex::index);
    
    if (lliurex::index == lliurex::groups.size()) {
        *errnop = ENOENT;
        return NSS_STATUS_NOTFOUND;
    }
    
    lliurex::Group& grp = lliurex::groups[lliurex::index];
    
    int status = push_group(grp,result,buffer,buflen);
    if (status == -1) {
        *errnop = ERANGE;
        return NSS_STATUS_TRYAGAIN;
    }
    
    lliurex::index++;
    return NSS_STATUS_SUCCESS;
}

/*
 * Find group entry by GID
*/
enum nss_status _nss_lliurex_getgrgid_r(gid_t gid, struct group* result, char* buffer, size_t buflen, int* errnop)
{
    std::lock_guard<std::mutex> lock(lliurex::mtx);
    
    sd_journal_print(LOG_INFO,"lliurex_getgrgid %d",gid);
    
    int db_status = update_db();
    if (db_status == -1) {
        *errnop = ENOENT;
        return NSS_STATUS_UNAVAIL;
    }
    
    for (lliurex::Group& grp : lliurex::groups) {
        if (grp.gid==gid) {
            sd_journal_print(LOG_INFO,"lliurex_getgrgid found!");
            int status = push_group(grp,result,buffer,buflen);
            if (status == -1) {
                *errnop = ERANGE;
                return NSS_STATUS_TRYAGAIN;
            }
            
            return NSS_STATUS_SUCCESS;
        }
    }
    
    // not found
    *errnop = ENOENT;
    return NSS_STATUS_NOTFOUND;
}

/*
 * Find group entry by name
*/
enum nss_status _nss_lliurex_getgrnam_r(const char* name, struct group* result, char *buffer, size_t buflen, int* errnop)
{
    std::lock_guard<std::mutex> lock(lliurex::mtx);
    
    sd_journal_print(LOG_INFO,"lliurex_getgrnam %s",name);
    
    int db_status = update_db();
    if (db_status == -1) {
        *errnop = ENOENT;
        return NSS_STATUS_UNAVAIL;
    }
    
    for (lliurex::Group& grp : lliurex::groups) {
        if (grp.name==std::string(name)) {
            sd_journal_print(LOG_INFO,"lliurex_getgrnam found!");
            int status = push_group(grp,result,buffer,buflen);
            if (status == -1) {
                *errnop = ERANGE;
                return NSS_STATUS_TRYAGAIN;
            }
            
            return NSS_STATUS_SUCCESS;
        }
    }
    
    // not found
    *errnop = ENOENT;
    return NSS_STATUS_NOTFOUND;
}
