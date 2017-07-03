#! /usr/bin/env python

# Copyright (c) 2013 Freescale Semiconductor, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code must retain the above copyright notice, this list
#   of conditions and the following disclaimer.
#
# o Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# o Neither the name of Freescale Semiconductor, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# # @brief Define 4 types bootloader
# 1) ROM-resident bootloader: boot from ROM, execute in ROM (fpga_bootloader)
kBootROM_ExecuteROM = 0x00

# 2) Flash-resident bootloader: boot from Flash, execute in Flash (freedom_bootloader / tower_bootloader)
kBootFlash_ExecuteFlash = 0x01

# 3) RAM-resident bootloader: boot from RAM, execute in RAM (flashloader)
kBootRAM_ExecuteRAM = 0x02

# 4) RAM-resident bootloader: boot from Flash, execute in RAM (flashloader_loader)
kBootFlash_ExecuteRAM = 0x03

# Boot Source Dictionary
# Used to allow command line params
bootSourcesDict = {
        # ROM-resident bootloader
        'rom_bootloader_fpga'    :  kBootROM_ExecuteROM,
        'rom_bootloader_tower'   :  kBootROM_ExecuteROM,
        'rom_bootloader_freedom' :  kBootROM_ExecuteROM,
        # flash-resident bootloader
        'freedom_bootloader'     :  kBootFlash_ExecuteFlash,
        'tower_bootloader'       :  kBootFlash_ExecuteFlash,
        'maps_bootloader'        :  kBootFlash_ExecuteFlash,
        # flashloader
        'flashloader'            :  kBootRAM_ExecuteRAM,
        'flashloader_loader'     :  kBootFlash_ExecuteRAM
}

