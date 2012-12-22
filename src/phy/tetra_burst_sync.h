#ifndef TETRA_BURST_SYNC_H
#define TETRA_BURST_SYNC_H

#include <stdint.h>

enum rx_state {
	RX_S_UNLOCKED,		/* we're completely unlocked */
	RX_S_KNOW_FSTART,	/* we know the next frame start */
	RX_S_LOCKED,		/* fully locked */
};

typedef void(*tetra_phy_rx_sync_cb_t)(enum rx_state state, void *ctx);

struct tetra_rx_state {
	enum rx_state state;
	unsigned int bits_in_buf;		/* how many bits are currently in bitbuf */
	uint8_t bitbuf[4096];
	unsigned int bitbuf_start_bitnum;	/* bit number at first element in bitbuf */
	unsigned int next_frame_start_bitnum;	/* frame start expected at this bitnum */

	void *phy_state;
	void *mac_state;

	void *ctx;
	tetra_phy_rx_sync_cb_t rx_sync_cb;
};

void tetra_rx_state_init(struct tetra_rx_state *trs);

/* input a raw bitstream into the tetra burst synchronizaer */
int tetra_burst_sync_in(struct tetra_rx_state *trs, uint8_t *bits, unsigned int len);

#endif /* TETRA_BURST_SYNC_H */
