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

#include <nss.h>

#include <iostream>
#include <mutex>

using namespace std;

/*
 * Open database
*/
enum nss_status _nss_lliurex_setgrent(int stayopen)
{
    enum nss_status ret;
    
    return ret;
}

/*
 * Close database
*/
enum nss_status _nss_lliurex_endgrent()
{
    enum nss_status ret;
    
    return ret;
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
