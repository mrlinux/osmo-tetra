/* (C) 2012 by Dimitri Stolnikov <horiz0n@gmx.net>
 * All Rights Reserved
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

#ifndef TETRA_H
#define TETRA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <osmocom/tetra/tetra_export.h>

typedef struct tetra_state tetra_state_t;

TETRA_API int tetra_alloc(tetra_state_t **state);

TETRA_API int tetra_free(tetra_state_t *state);

TETRA_API int tetra_tmo_bits_input(tetra_state_t *state,
				   uint8_t *bits,
				   unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* TETRA_H */
