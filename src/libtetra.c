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

#include <string.h>

#include <osmocom/core/utils.h>
#include <osmocom/core/talloc.h>

#include "tetra_common.h"
#include <phy/tetra_burst.h>
#include <phy/tetra_burst_sync.h>
#include "tetra_gsmtap.h"

#ifdef HAVE_TETRA_CODEC
#include <osmocom/tetra/codec_acelp.h>
#endif

#include "osmocom/tetra/tetra.h"

void *tetra_tall_ctx;

static void tetra_phy_rx_sync_cb(void *ctx, enum rx_state state)
{
	tetra_state_t *ts = ctx;

	if (ts->rx_sync_cb)
		ts->rx_sync_cb(ts->ctx, RX_S_LOCKED == state ? 1 : 0);
}

static void tetra_mac_sys_info_cb(void *ctx, struct tetra_si_decoded *si)
{
	tetra_state_t *ts = ctx;
	tetra_sysinfo_t sysinfo;

	sysinfo.dl_freq = tetra_dl_carrier_hz(si->freq_band,
				      si->main_carrier,
				      si->freq_offset);

	sysinfo.ul_freq = tetra_ul_carrier_hz(si->freq_band,
				      si->main_carrier,
				      si->freq_offset,
				      si->duplex_spacing,
				      si->reverse_operation);

	sysinfo.serv_det = si->mle_si.bs_service_details;

	if (ts->sys_info_cb)
		ts->sys_info_cb(ts->ctx, &sysinfo);
}

static void tetra_mac_cell_data_cb(void *ctx, struct tetra_cell_data *cd)
{
	tetra_state_t *ts = ctx;
	tetra_celldata_t celldata;

	celldata.mcc = cd->mcc;
	celldata.mnc = cd->mnc;
	celldata.colour_code = cd->colour_code;

	if (ts->cell_data_cb)
		ts->cell_data_cb(ts->ctx, &celldata);
}

#ifdef HAVE_TETRA_CODEC
static void tetra_mac_traffic_cb(void *ctx,
				 const uint8_t *bits, unsigned int len,
				 uint32_t tn, uint32_t dl_usage, uint32_t ssi )
{
	tetra_state_t *ts = ctx;
	int16_t block[432];
	int16_t voice[480];
	int i;

	for (i=0; i<432; i++)
		block[i] = bits[i] ? -127 : 127;

	tetra_acelp_decode_frame(block, voice);

	if (ts->traffic_cb)
		ts->traffic_cb(ts->ctx, voice, 480, tn, dl_usage, ssi);
}
#endif

int tetra_alloc(tetra_state_t **state)
{
	tetra_state_t *ts;
	struct tetra_rx_state *trs;
	struct tetra_phy_state *tps;
	struct tetra_mac_state *tms;

	if (!state)
		return -1;

	tetra_gsmtap_init("localhost", 0);

	ts = talloc_zero(tetra_tall_ctx, tetra_state_t);
	memset(ts, 0, sizeof(tetra_state_t));

	tps = talloc_zero(tetra_tall_ctx, struct tetra_phy_state);
	tetra_phy_state_init(tps);

	tps->ctx = ts;
	tps->rx_sync_cb = tetra_phy_rx_sync_cb;

	tms = talloc_zero(tetra_tall_ctx, struct tetra_mac_state);
	tetra_mac_state_init(tms);

	tms->ctx = ts;
	tms->sys_info_cb = tetra_mac_sys_info_cb;
	tms->cell_data_cb = tetra_mac_cell_data_cb;
#ifdef HAVE_TETRA_CODEC
	tetra_acelp_decode_init();
	tms->traffic_cb = tetra_mac_traffic_cb;
#endif
	trs = talloc_zero(tetra_tall_ctx, struct tetra_rx_state);
	tetra_rx_state_init(trs);

	trs->phy_state = tps;
	trs->mac_state = tms;

	ts->trs = trs;

	*state = ts;

	return 0;
}

int tetra_free(tetra_state_t *state)
{
	struct tetra_rx_state *trs;

	if (!state)
		return -1;

	trs = state->trs;

	talloc_free(trs->phy_state);
	talloc_free(trs->mac_state);
	talloc_free(state->trs);
	talloc_free(state);

	return 0;
}

int tetra_tmo_bits_input(tetra_state_t *state, uint8_t *bits, unsigned int len)
{
	if (!state)
		return -1;

	return tetra_burst_sync_in(state->trs, bits, len);
}
