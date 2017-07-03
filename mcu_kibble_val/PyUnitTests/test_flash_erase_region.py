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

import pytest
import os
import sys
import random

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util


## @breif Verify flash-erase-region command according to given parameters.
# @param: bl
#         length         byte/sector count to be erased
#         lengthType     'Bytes'                   Erase 'length' bytes flash
#                        'Sectors'                 Erase 'length' sectors will be erased
#                        'OneSectorPlusOneByte'    Erase 'length' * (1 sector + 1) bytes
#                        'HalfMemory'              Erase 'length' * half_flash_size bytes
#                        'AllMemory'               Erase whole flash
#         locationType   'StartOfMemory'           Erase flash from the start address
#                        'EndOfMemory'             Erase flash from the (end address - actual length + 1)
def flash_erase_region_according_to_parameters(bl, length, lengthType, locationType):
    # Get actual start address and byte count to be filled
    startAddress, actualLength = common_util.get_start_address_and_length(bl, length, lengthType, 'flash', locationType)
    status, results = bl.flash_erase_region(startAddress, actualLength)
    # Get the location of a given block
    locationStatus = common_util.block_location(bl, startAddress, actualLength, 'flash')
    if locationStatus == common_util.kInvalidParameter:
        assert status != bootloader.status.kStatus_Success
    elif locationStatus == common_util.kZeroSizeBlock:
        assert status == bootloader.status.kStatus_Success
    elif locationStatus == common_util.kInvalidMemoryRange:
        assert status == bootloader.status.kStatusMemoryRangeInvalid
    elif locationStatus == common_util.kValidMemoryRange:
        if startAddress % bl.target.eraseAlignmentSize == 0 and actualLength % bl.target.eraseAlignmentSize == 0:
            assert status == bootloader.status.kStatus_Success
        elif startAddress % bl.target.eraseAlignmentSize > 0 or actualLength % bl.target.eraseAlignmentSize > 0:
            assert status == bootloader.status.kStatus_FlashAlignmentError

class Testflash_erase_region:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_erase_bytes_at_start_of_flash(self, bl, bytesNumber):
        flash_erase_region_according_to_parameters(bl, bytesNumber, 'Bytes', 'StartOfMemory')

    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_erase_bytes_at_end_of_flash(self, bl, bytesNumber):
        flash_erase_region_according_to_parameters(bl, bytesNumber, 'Bytes', 'EndOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_erase_sectors_at_start_of_flash(self, bl, sectorsNumber):
        flash_erase_region_according_to_parameters(bl, sectorsNumber, 'Sectors', 'StartOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_erase_sectors_at_end_of_flash(self, bl, sectorsNumber):
        flash_erase_region_according_to_parameters(bl, sectorsNumber, 'Sectors', 'EndOfMemory')

    def test_erase_one_sector_plus_one_byte_at_start_of_flash(self, bl):
        flash_erase_region_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'StartOfMemory')

    def test_erase_one_sector_plus_one_byte_at_end_of_flash(self, bl):
        flash_erase_region_according_to_parameters(bl, 1, 'OneSectorPlusOneByte','EndOfMemory')

    def test_erase_half_of_flash(self, bl):
        flash_erase_region_according_to_parameters(bl, 1, 'HalfMemory', 'StartOfMemory')

    def test_erase_all_of_flash(self, bl):
        flash_erase_region_according_to_parameters(bl, 1, 'AllMemory', 'StartOfMemory')

    def test_erase_unaligned_flash_address(self, bl):
        common_util.verify_manipulate_unaligned_address_according_to_parameters(bl, 'flash-erase-region', 'flash')

    def test_erase_unaligned_flash_length(self, bl):
        common_util.verify_manipulate_unaligned_length_according_to_parameters(bl, 'flash-erase-region', 'flash')

    def test_erase_region_out_of_flash(self, bl):
        address = common_util.get_memory_start_address(bl, 'flash') + common_util.get_memory_total_size(bl, 'flash')
        bytesNumber = common_util.get_flash_sector_size(bl)
        status, results = bl.flash_erase_region(address, bytesNumber)
        assert status == bootloader.status.kStatusMemoryRangeInvalid

    def test_erase_all_available_flash(self, bl):
        availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, 'flash')
        # 1. erase all the flash
        status, results = bl.flash_erase_all()
        assert status == bootloader.status.kStatus_Success
        # 2. write random data to [availableRegionStartAddress, availableRegionEndAddress]
        randomFile = common_util.generate_random_data_file(bl, availableRegionStartAddress, availableRegionSize)
        status, results = bl.write_memory(availableRegionStartAddress, randomFile)
        assert status == bootloader.status.kStatus_Success
        # 3. erase flash
        status, results = bl.flash_erase_region(availableRegionStartAddress, availableRegionSize)
        assert status == bootloader.status.kStatus_Success
        # 4. read data from [availableRegionStartAddress, availableRegionEndAddress]
        filePath = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results == bl.read_memory(availableRegionStartAddress, availableRegionSize, filePath)
        assert status == bootloader.status.kStatus_Success
        # 5. verify all the data are FF
        flag = True
        fileObj = open(filePath, 'rb')
        for i in range(0, availableRegionSize):
           fileObj.seek(i)
           data_tmp = fileObj.read(1)
           if ord(data_tmp) != 0xFF:
               flag = False
               break
        fileObj.close()
        assert flag == True

