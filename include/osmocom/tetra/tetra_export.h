/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRA_EXPORT_H
#define TETRA_EXPORT_H

#if defined __GNUC__
#  if __GNUC__ >= 4
#    define __TETRA_EXPORT   __attribute__((visibility("default")))
#    define __TETRA_IMPORT   __attribute__((visibility("default")))
#  else
#    define __TETRA_EXPORT
#    define __TETRA_IMPORT
#  endif
#elif _MSC_VER
#  define __TETRA_EXPORT     __declspec(dllexport)
#  define __TETRA_IMPORT     __declspec(dllimport)
#else
#  define __TETRA_EXPORT
#  define __TETRA_IMPORT
#endif

#ifndef osmotetra_STATIC
#	ifdef osmotetra_EXPORTS
#	define TETRA_API __TETRA_EXPORT
#	else
#	define TETRA_API __TETRA_IMPORT
#	endif
#else
#define TETRA_API
#endif
#endif /* TETRA_EXPORT_H */
