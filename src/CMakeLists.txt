# Copyright 2012 OSMOCOM Project
#
# This file is part of osmo-tetra
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

########################################################################
# Setup library
########################################################################

add_library(osmotetra-phy STATIC
    phy/tetra_burst_sync.c
    phy/tetra_burst.c
)

set_target_properties(osmotetra-phy PROPERTIES COMPILE_FLAGS "-fPIC")

add_library(osmotetra-mac STATIC
    lower_mac/tetra_conv_enc.c
    lower_mac/tch_reordering.c
    lower_mac/tetra_scramb.c
    lower_mac/tetra_rm3014.c
    lower_mac/tetra_interleave.c
    lower_mac/crc_simple.c
    lower_mac/viterbi.c
    lower_mac/viterbi_cch.c
    lower_mac/viterbi_tch.c
    lower_mac/tetra_lower_mac.c
    tetra_tdma.c
    tetra_common.c
    tetra_upper_mac.c
    tetra_mac_pdu.c
    tetra_llc_pdu.c
    tetra_llc.c
    tetra_mle_pdu.c
    tetra_mm_pdu.c
    tetra_cmce_pdu.c
    tetra_sndcp_pdu.c
    tetra_gsmtap.c
    tuntap.c
)

add_library(osmotetra SHARED
    libtetra.c
)

set_target_properties(osmotetra-mac PROPERTIES COMPILE_FLAGS "-fPIC")

target_link_libraries(osmotetra osmotetra-phy osmotetra-mac ${LIBOSMOCORE_LIBRARIES})
if(LIBTETRACODEC_FOUND)
    target_link_libraries(osmotetra ${LIBTETRACODEC_LIBRARIES})
endif()

set_target_properties(osmotetra PROPERTIES DEFINE_SYMBOL "osmotetra_EXPORTS")
set_target_properties(osmotetra PROPERTIES OUTPUT_NAME osmotetra)
set_target_properties(osmotetra PROPERTIES SOVERSION 0 VERSION 0.0.0)

########################################################################
# Build utility
########################################################################
add_executable(crc_test crc_test.c)
target_link_libraries(crc_test osmotetra-mac ${LIBOSMOCORE_LIBRARIES})

add_executable(conv_enc_test conv_enc_test.c testpdu.c)
target_link_libraries(conv_enc_test osmotetra-phy osmotetra-mac ${LIBOSMOCORE_LIBRARIES})

add_executable(tunctl tunctl.c)

add_executable(float_to_bits float_to_bits.c)

add_executable(tetra-rx tetra-rx.c)
target_link_libraries(tetra-rx osmotetra)

set(INSTALL_TARGETS osmotetra float_to_bits tetra-rx)

########################################################################
# Install built library files & utilities
########################################################################
install(TARGETS ${INSTALL_TARGETS}
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)
