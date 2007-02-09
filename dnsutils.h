/*w [@dnsutils.h@]
* Copyright (C) 2007 Roberto Alsina <ralsina@kde.org>
*
* For any questions please contact Roberto Alsina, because
* this version is heavily modified from the original.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*/

#ifndef DNS_UTILS_H
#define DNS_UTILS_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include "bstrlib.h"
#include "udns/udns.h"

int checkrbl (bstring lookup_addr, const char *rbl);

bstring describerbl (bstring lookup_addr, const char *rbl);

int mailservers (bstring domain, struct bstrList **list);

#endif
