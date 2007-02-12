/*w [@vpoputils.h@]
 * Copyright © 2006 Roberto Alsina <ralsina@kde.org>
 *
 * For any questions please contact Roberto Alsina, because
 * this version is heavily modified from Mr. Foremski's original.
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
#ifndef VPOP_UTILS_H
#define VPOP_UTILS_H

#include "bstrlib.h"

// Utility functions to deal with vpopmail
// 1 if domain is local, 0 if not

int isDomainVirtual (bstring domain);
int isSubAddress (bstring username, bstring domain);
int isAlias (bstring username, bstring domain);
int isUser (bstring username, bstring domain);
int quotaUsage (bstring username, bstring domain);

#endif
