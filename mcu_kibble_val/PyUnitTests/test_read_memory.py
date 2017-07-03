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

import sys
import os
import time

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util

## @breif Verify read-memory according to the given parameters.
# @param: bl
#         length         byte/sector count to be filled with
#         lengthType     'Bytes'                   'length' bytes will be read from the given address
#                        'Sectors'                 'length' sectors will be read from the given address
#                        'OneSectorPlusOneByte'    'length' * (1 sector + 1) bytes will be read from the given address
#                        'HalfMemory'              'length' * half_flash_size bytes will be read from the given address
#                        'AllMemory'               whole memory will be read from the given address
#         memType        'flash'             read from flash array
#                        'ram'               read from ram
#         locationType   'StartOfMemory'     read from the start address of given memory (flash or ram)
#                        'EndOfMemory'       read from end address - actual length + 1
def read_memory_according_to_parameters(bl, length, lengthType, memType, locationType):
    # 1. Get actual start address and byte count to be read
    startAddress, actualLength = common_util.get_start_address_and_length(bl, length, lengthType, memType, locationType)
    # 2. Read data from memory by read-memory command.
    command_read_data_file = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
    status, results = bl.read_memory(startAddress, actualLength, command_read_data_file)
    # 3. Give the return value according to parameters.
    memoryStartAddress, memoryEndAddress = common_util.get_memory_start_end_address(bl, memType)
    endAddress = startAddress + actualLength - 1
    if actualLength == 0:
        assert status == bootloader.status.kStatus_Success
    elif ((startAddress >= memoryStartAddress) and (endAddress <= memoryEndAddress)):
        assert status == bootloader.status.kStatus_Success
    else:
        assert status == bootloader.status.kStatusMemoryRangeInvalid

# @brief Test cases for read-memory command.
class TestReadFlash:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('byteCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_bytes_at_start_of_flash(self, bl, byteCount):
        read_memory_according_to_parameters(bl, byteCount, 'Bytes', 'flash', 'StartOfMemory')

    @pytest.mark.parametrize(('byteCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_bytes_at_end_of_flash(self, bl, byteCount):
        read_memory_according_to_parameters(bl, byteCount, 'Bytes', 'flash', 'EndOfMemory')

    @pytest.mark.parametrize(('sectorCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_sectors_at_start_of_flash(self, bl, sectorCount):
        read_memory_according_to_parameters(bl, sectorCount, 'Sectors', 'flash', 'StartOfMemory')

    @pytest.mark.parametrize(('sectorCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_sectors_at_end_of_flash(self, bl, sectorCount):
        read_memory_according_to_parameters(bl, sectorCount, 'Sectors', 'flash', 'EndOfMemory')

    def test_read_one_sector_plus_one_byte_at_start_of_flash(self, bl):
        read_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'flash', 'StartOfMemory')

    def test_read_one_sector_plus_one_byte_at_end_of_flash(self, bl):
        read_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'flash', 'EndOfMemory')

    def test_read_half_of_flash(self, bl):
        read_memory_according_to_parameters(bl, 1, 'HalfMemory', 'flash', 'EndOfMemory')

    def test_read_all_of_flash(self, bl):
        read_memory_according_to_parameters(bl, 1, 'AllMemory', 'flash', 'StartOfMemory')

    def test_read_unaligned_flash_address(self, bl):
        common_util.verify_manipulate_unaligned_address_according_to_parameters(bl, 'read-memory', 'flash')


class TestReadRam:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('byteCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_bytes_at_start_of_ram(self, bl, byteCount):
        read_memory_according_to_parameters(bl, byteCount, 'Bytes', 'ram', 'StartOfMemory')

    @pytest.mark.parametrize(('byteCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_bytes_at_end_of_ram(self, bl, byteCount):
        read_memory_according_to_parameters(bl, byteCount, 'Bytes', 'ram', 'EndOfMemory')

    @pytest.mark.parametrize(('sectorCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_sectors_at_start_of_ram(self, bl, sectorCount):
        read_memory_according_to_parameters(bl, sectorCount, 'Sectors', 'ram', 'StartOfMemory')

    @pytest.mark.parametrize(('sectorCount'), [
                             (0),
                             (1),
                             (2),
                             (3),
                             (8)
                            ])
    def test_read_sectors_at_end_of_ram(self, bl, sectorCount):
        read_memory_according_to_parameters(bl, sectorCount, 'Sectors', 'ram', 'EndOfMemory')

    def test_read_one_sector_plus_one_byte_at_start_of_ram(self, bl):
        read_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'ram', 'StartOfMemory')

    def test_read_one_sector_plus_one_byte_at_end_of_ram(self, bl):
        read_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'ram', 'EndOfMemory')

    def test_read_half_of_ram(self, bl):
        read_memory_according_to_parameters(bl, 1, 'HalfMemory', 'ram', 'EndOfMemory')

    def test_read_all_of_ram(self, bl):
        read_memory_according_to_parameters(bl, 1, 'AllMemory', 'ram', 'StartOfMemory')

    def test_read_unaligned_ram_address(self, bl):
        common_util.verify_manipulate_unaligned_address_according_to_parameters(bl, 'read-memory', 'ram')

class TestReadInvalidRegion:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_read_data_from_invalid_flash_address(self, bl):
        # Set start address as the end address of flash which should be an invalid address
        flashStartAddr = common_util.get_memory_start_address(bl, 'flash')
        flashTotalSize = common_util.get_memory_total_size(bl, 'flash')
        startReadAddr = flashStartAddr + flashTotalSize
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startReadAddr, 32, readFile)
        assert status == bootloader.status.kStatusMemoryRangeInvalid

    def test_read_data_from_invalid_ram_address(self, bl):
        # Set start address as the end address of ram which should be an invalid address
        ramStartAddr = common_util.get_memory_start_address(bl, 'ram')
        ramTotalSize = common_util.get_memory_total_size(bl, 'ram')
        startReadAddr = ramStartAddr + ramTotalSize
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startReadAddr, 32, readFile)
        assert status == bootloader.status.kStatusMemoryRangeInvalid

    def test_read_data_out_of_flash_range(self, bl):
        # Set start address as a valid flash address, but the end address is out of valid flash region
        flashStartAddr = common_util.get_memory_start_address(bl, 'flash')
        flashTotalSize = common_util.get_memory_total_size(bl, 'flash')
        startReadAddr = flashStartAddr + flashTotalSize - 1
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startReadAddr, 32, readFile)
        assert status == bootloader.status.kStatusMemoryRangeInvalid

    def test_read_data_out_of_ram_range(self, bl):
        # Set start address as a valid ram address, but the end address is out of valid ram region
        ramStartAddr = common_util.get_memory_start_address(bl, 'ram')
        ramTotalSize = common_util.get_memory_total_size(bl, 'ram')
        startReadAddr = ramStartAddr + ramTotalSize - 1
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startReadAddr, 32, readFile)
        assert status == bootloader.status.kStatusMemoryRangeInvalid
