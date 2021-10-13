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

#include <iostream>
#include <string>
#include <mutex>

using namespace edupals;
using namespace std;

struct Group
{
    std::string name;
    uint64_t gid;
    std::vector<std::string> members;
};

std::mutex mtx;
std::vector<Group> groups;
int index = -1;

/*
 * Open database
*/
enum nss_status _nss_lliurex_setgrent(int stayopen)
{
    std::lock_guard<std::mutex> lock(mtx);
    
    index = -1;
    
    n4d::Client client;
    
    try {
        variant::Variant ret = client.call("CDC","getgrall");
        
        groups.clear();
        
        for (string key : ret.keys()) {
            Group grp;
            
            grp.name = key;
            grp.gid = ret[key][0];
            
            for (size_t n=0;n<ret[key][1].count();n++) {
                grp.members.push_back(ret[key][1][n].get_string());
            }
            
            groups.push_back(grp);
        }
    }
    catch (std::exception& e) {
        return NSS_STATUS_UNAVAIL;
    }
    
    index = 0;
    
    return NSS_STATUS_SUCCESS;
}

/*
 * Close database
*/
enum nss_status _nss_lliurex_endgrent()
{
    return NSS_STATUS_SUCCESS;
}

/*
 * Read group entry
*/
enum nss_status _nss_lliurex_getgrent_r(struct group *result, char *buffer, size_t buflen, int *errnop)
{
}

/*
 * Find group entry by GID
*/
enum nss_status _nss_lliurex_getgrgid_r(gid_t gid, struct group *result, char *buffer, size_t buflen, int *errnop)
{
}

/*
 * Find group entry by name
*/
enum nss_status _nss_lliurex_getgrnam_r(const char *name, struct group *result, char *buffer, size_t buflen, int *errnop)
{
}
