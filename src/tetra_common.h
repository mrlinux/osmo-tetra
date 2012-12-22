#ifndef TETRA_COMMON_H
#define TETRA_COMMON_H

#include <stdint.h>
#include "tetra_mac_pdu.h"
#include <osmocom/core/linuxlist.h>

#ifdef DEBUG
#define DEBUGP(x, args...)	fprintf(stderr, x, ## args)
#else
#define DEBUGP(x, args...)	do { } while(0)
#endif

#define TETRA_SYM_PER_TS	255
#define TETRA_BITS_PER_TS	(TETRA_SYM_PER_TS*2)

/* Chapter 22.2.x */
enum tetra_log_chan {
	TETRA_LC_UNKNOWN,
	/* TMA SAP */
	TETRA_LC_SCH_F,
	TETRA_LC_SCH_HD,
	TETRA_LC_SCH_HU,
	TETRA_LC_STCH,
	TETRA_LC_SCH_P8_F,
	TETRA_LC_SCH_P8_HD,
	TETRA_LC_SCH_P8_HU,

	TETRA_LC_AACH,
	TETRA_LC_TCH,
	TETRA_LC_BSCH,
	TETRA_LC_BNCH,

	/* FIXME: QAM */
};

uint32_t bits_to_uint(const uint8_t *bits, unsigned int len);

#include "tetra_tdma.h"

enum rx_state {
	RX_S_UNLOCKED,		/* we're completely unlocked */
	RX_S_KNOW_FSTART,	/* we know the next frame start */
	RX_S_LOCKED,		/* fully locked */
};

typedef void(*tetra_phy_rx_sync_cb_t)(void *ctx, enum rx_state state);

struct tetra_phy_state {
	struct tetra_tdma_time time;

	void *ctx;
	tetra_phy_rx_sync_cb_t rx_sync_cb;
};
extern struct tetra_phy_state t_phy_state;

void tetra_phy_state_init(struct tetra_phy_state *tps);

typedef void(*tetra_mac_sys_info_cb_t)(void *ctx, struct tetra_si_decoded *si);

typedef void(*tetra_mac_cell_data_cb_t)(void *ctx, struct tetra_cell_data *cd );

typedef void(*tetra_mac_traffic_cb_t)(void *ctx,
				      const uint8_t *bits, unsigned int len,
				      uint32_t tn, uint32_t dl_usage, uint32_t ssi);

struct tetra_mac_state {
	struct llist_head voice_channels;
	struct {
		uint32_t dl_usage;
		uint32_t ssi;
	} cur_burst;
	struct tetra_si_decoded last_sid;

	void *ctx;
	tetra_mac_sys_info_cb_t sys_info_cb;
	tetra_mac_cell_data_cb_t cell_data_cb;
	tetra_mac_traffic_cb_t traffic_cb;
};

void tetra_mac_state_init(struct tetra_mac_state *tms);

#define TETRA_CRC_OK	0x1d0f

uint32_t tetra_dl_carrier_hz(uint8_t band, uint16_t carrier, uint8_t offset);
uint32_t tetra_ul_carrier_hz(uint8_t band, uint16_t carrier, uint8_t offset,
			     uint8_t duplex, uint8_t reverse);

const char *tetra_get_lchan_name(enum tetra_log_chan lchan);
const char *tetra_get_sap_name(uint8_t sap);
#endif
