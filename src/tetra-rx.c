/* Test program for tetra burst synchronizer */

/* (C) 2011 by Harald Welte <laforge@gnumonks.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <osmocom/core/utils.h>
#include <osmocom/core/talloc.h>

#include "tetra_common.h"
#include <phy/tetra_burst.h>
#include <phy/tetra_burst_sync.h>
#include "tetra_gsmtap.h"

#ifdef HAVE_TETRA_CODEC
#include <osmocom/tetra/codec_acelp.h>
#endif

void *tetra_tall_ctx;

static void tetra_phy_rx_sync_cb(void *ctx, enum rx_state state)
{
	if (RX_S_LOCKED == state)
		fprintf(stderr, "receiver synchronized.\n");
	else
		fprintf(stderr, "receiver lost synchro.\n");
}

static void tetra_mac_sys_info_cb(void *ctx, struct tetra_si_decoded *si)
{
	uint32_t dl_freq, ul_freq;

	dl_freq = tetra_dl_carrier_hz(si->freq_band,
				      si->main_carrier,
				      si->freq_offset);

	ul_freq = tetra_ul_carrier_hz(si->freq_band,
				      si->main_carrier,
				      si->freq_offset,
				      si->duplex_spacing,
				      si->reverse_operation);

	fprintf(stderr, "sysinfo (DL %u Hz, UL %u Hz), serv_det 0x%04x",
		dl_freq, ul_freq, si->mle_si.bs_service_details);

	fprintf(stderr, " air_encr %s",
		si->mle_si.bs_service_details & BS_SERVDET_AIR_ENCR ? "yes" : "no");

	if (si->cck_valid_no_hf)
		fprintf(stderr, " cck id %u", si->cck_id);
	else
		fprintf(stderr, " hframe %u", si->hyperframe_number);

	fprintf(stderr, "\n");
}

static void tetra_mac_cell_data_cb(void *ctx, struct tetra_cell_data *cd)
{
	fprintf(stderr, "sync MNC %u MNC %u CC 0x%02x\n",
		cd->mcc, cd->mnc, cd->colour_code);
}

#ifdef HAVE_TETRA_CODEC
static void tetra_mac_traffic_cb(void *ctx,
				 const uint8_t *bits, unsigned int len,
				 uint32_t tn, uint32_t dl_usage, uint32_t ssi )
{
	char fname[100];
	int16_t block[432];
	int16_t voice[480];
	FILE *file;
	int i;

	/* Open target file */
	snprintf(fname, 100, "/tmp/traffic_%d.pcm", dl_usage);
	file = fopen(fname, "ab");

	for (i=0; i<432; i++)
		block[i] = bits[i] ? -127 : 127;

	tetra_acelp_decode_frame(block, voice);

	/* Write it */
	fwrite(voice, sizeof(int16_t), 480, file);

	/* Close */
	fclose(file);
}
#endif

int main(int argc, char **argv)
{
	int fd;
	struct tetra_rx_state *trs;
	struct tetra_phy_state *tps;
	struct tetra_mac_state *tms;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file_with_1_byte_per_bit>\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(2);
	}

	tetra_gsmtap_init("localhost", 0);

	tps = talloc_zero(tetra_tall_ctx, struct tetra_phy_state);
	tetra_phy_state_init(tps);

	tps->rx_sync_cb = tetra_phy_rx_sync_cb;

	tms = talloc_zero(tetra_tall_ctx, struct tetra_mac_state);
	tetra_mac_state_init(tms);

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

	while (1) {
		uint8_t buf[64];
		int len;

		len = read(fd, buf, sizeof(buf));
		if (len < 0) {
			perror("read");
			exit(1);
		} else if (len == 0) {
			fprintf(stderr, "EOF");
			break;
		}
		tetra_burst_sync_in(trs, buf, len);
	}

	talloc_free(trs);
	talloc_free(tps);
	talloc_free(tms);

	exit(0);
}
