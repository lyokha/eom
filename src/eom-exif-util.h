/* Eye Of Mate - EXIF Utilities
 *
 * Copyright (C) 2006-2007 The Free Software Foundation
 *
 * Author: Lucas Rocha <lucasr@gnome.org>
 * Author: Claudio Saavedra <csaavedra@alumnos.utalca.cl>
 * Author: Felix Riemann <felix@hsgheli.de>
 *
 * Based on code by:
 *	- Jens Finke <jens@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __EOM_EXIF_UTIL_H__
#define __EOM_EXIF_UTIL_H__

#include <glib.h>
#include <libexif/exif-data.h>

G_BEGIN_DECLS

gchar*       eom_exif_util_format_date           (const gchar *date);

const gchar *eom_exif_util_get_value             (ExifData *exif_data, gint tag_id, gchar *buffer, guint buf_size);

G_END_DECLS

#endif /* __EOM_EXIF_UTIL_H__ */
