#! /usr/bin/env python

# Copyright (c) 2014 Freescale Semiconductor, Inc.
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

import os

# @breif Fill the specific 8-bit data to bytearray.
def fill_data_8bits(byteArray, data8bits):
    byteArray.append(data8bits)

# @breif Generate the flash config data
def generate_flash_config_data(
                            # 0x400 ~ 0x407
                            backdoorKeyByte0       =   0xff,
                            backdoorKeyByte1       =   0xff,
                            backdoorKeyByte2       =   0xff,
                            backdoorKeyByte3       =   0xff,
                            backdoorKeyByte4       =   0xff,
                            backdoorKeyByte5       =   0xff,
                            backdoorKeyByte6       =   0xff,
                            backdoorKeyByte7       =   0xff,
                            # 0x408 ~ 0x40B (Refer to the description of the Program Flash Protection Register, FPROT0-3)
                            flashProtectionByte0   =   0xff,
                            flashProtectionByte1   =   0xff,
                            flashProtectionByte2   =   0xff,
                            flashProtectionByte3   =   0xff,
                            # 0x40C (Refer to the description of the Flash Security Register, FSEC)
                            flashSecurityRegister  =   0xfe,
                            # 0x40D (Refer to the description of the Flash Option Register, FOPT)
                            flashOptionRegister    =   0xff,
                            # 0x40E ~ 0x40F
                            reservedByte0          =   0xff,
                            reservedByte1          =   0xff
                             ):
    flashConfigData = bytearray()
    fill_data_8bits(flashConfigData, backdoorKeyByte0)
    fill_data_8bits(flashConfigData, backdoorKeyByte1)
    fill_data_8bits(flashConfigData, backdoorKeyByte2)
    fill_data_8bits(flashConfigData, backdoorKeyByte3)
    fill_data_8bits(flashConfigData, backdoorKeyByte4)
    fill_data_8bits(flashConfigData, backdoorKeyByte5)
    fill_data_8bits(flashConfigData, backdoorKeyByte6)
    fill_data_8bits(flashConfigData, backdoorKeyByte7)
    fill_data_8bits(flashConfigData, flashProtectionByte0)
    fill_data_8bits(flashConfigData, flashProtectionByte1)
    fill_data_8bits(flashConfigData, flashProtectionByte2)
    fill_data_8bits(flashConfigData, flashProtectionByte3)
    fill_data_8bits(flashConfigData, flashSecurityRegister)
    fill_data_8bits(flashConfigData, flashOptionRegister)
    fill_data_8bits(flashConfigData, reservedByte0)
    fill_data_8bits(flashConfigData, reservedByte1)
    # Write flash config data to the specific file.
    flashConfigDataFile = os.path.join(os.path.dirname(__file__), '..', 'PyUnitTests', 'working', 'vectors', 'flashConfigData.dat')
    fileObj = open(flashConfigDataFile, 'wb+')
    fileObj.write(flashConfigData)
    fileObj.close()
    return flashConfigDataFile

