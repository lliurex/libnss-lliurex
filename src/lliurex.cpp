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

#include <n4d.hpp>

#include <nss.h>
#include <systemd/sd-journal.h>

#include <iostream>
#include <string>
#include <mutex>

using namespace edupals;
using namespace std;

namespace lliurex
{
    struct Group
    {
        std::string name;
        uint64_t gid;
        std::vector<std::string> members;
    };
}

std::mutex mtx;
std::vector<lliurex::Group> groups;
int index = -1;

static int push_string(string in,char** buffer, size_t* remain)
{
    size_t fsize = in.size() + 1;
    if (fsize > *remain) {
        return -1;
    }
    
    std::memcpy(*buffer,in.c_str(),fsize);
    
    *remain -= fsize;
    *buffer += fize;
    
    return 0;
}

/*
 * Open database
*/
enum nss_status _nss_lliurex_setgrent(int stayopen)
{
    std::lock_guard<std::mutex> lock(mtx);
    sd_journal_print(LOG_INFO,"lliurex_setgrent...");
    
    index = -1;
    
    n4d::Client client;
    
    try {
        variant::Variant ret = client.call("CDC","getgrall");
        
        groups.clear();
        
        for (string key : ret.keys()) {
            lliurex::Group grp;
            
            grp.name = key;
            grp.gid = ret[key][0];
            
            for (size_t n=0;n<ret[key][1].count();n++) {
                grp.members.push_back(ret[key][1][n].get_string());
            }
            
            groups.push_back(grp);
        }
    }
    catch (std::exception& e) {
        sd_journal_print(LOG_ERR,"lliurex_setgrent: %s",e.what());
        return NSS_STATUS_UNAVAIL;
    }
    
    index = 0;
    sd_journal_print(LOG_INFO,"lliurex_setgrent successful");
    return NSS_STATUS_SUCCESS;
}

/*
 * Close database
*/
enum nss_status _nss_lliurex_endgrent()
{
    sd_journal_print(LOG_INFO,"lliurex_endgrent");
    return NSS_STATUS_SUCCESS;
}

/*
 * Read group entry
*/
enum nss_status _nss_lliurex_getgrent_r(struct group* result, char* buffer, size_t buflen, int* errnop)
{
    std::lock_guard<std::mutex> lock(mtx);
    sd_journal_print(LOG_INFO,"lliurex_getgrent %d",index);
    
    if (index == groups.size()) {
        *errno = NOENT;
        return NSS_STATUS_NOTFOUND;
    }
    
    char* ptr = buffer;
    
    lliurex::Group& grp = groups[index];
    
    result->gr_gid = grp.gid;
    
    if (push_string(grp.name,&ptr,&buflen) == -1) {
        *errnop = ERANGE;
        return NSS_STATUS_TRYAGAIN;
    }
    
    result->gr_name = ptr;
    vector<char*> tmp;
    
    for (string member : grp.members) {
        if (push_string(member,&ptr,&buflen) == -1) {
            *errnop = ERANGE;
            return NSS_STATUS_TRYAGAIN;
        }
        tmp.push_back(ptr);
    }
    
    if ( (sizeof(char*)*(tmp.size()+1)) > buflen) {
        *errnop = ERANGE;
        return NSS_STATUS_TRYAGAIN;
    }
    
    result->gr_mem = &ptr;
    
    int n = 0;
    
    for (n = 0;n<tmp.size();n++) {
        result->gr_mem[n] = tmp[n];
    }
    
    result->gr_mem[n] = 0;
    
    index++;
    return NSS_STATUS_SUCCESS;
}

/*
 * Find group entry by GID
*/
enum nss_status _nss_lliurex_getgrgid_r(gid_t gid, struct group* result, char* buffer, size_t buflen, int* errnop)
{
    std::lock_guard<std::mutex> lock(mtx);
}

/*
 * Find group entry by name
*/
enum nss_status _nss_lliurex_getgrnam_r(const char* name, struct group* result, char *buffer, size_t buflen, int* errnop)
{
    std::lock_guard<std::mutex> lock(mtx);
}
