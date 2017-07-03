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

## @breif Verify write-memory command according to given parameters.
# @param: bl
#         length         byte/sector
#         lengthType     'Bytes'
#                        'Sectors'
#                        'OneSectorPlusOneByte'
#                        'HalfMemory'
#                        'AllMemory'
#         memType        'flash'
#                        'ram'
#         locationType   'StartOfMemory'
#                        'EndOfMemory'
def write_memory_according_to_parameters(bl, length, lengthType, memType, locationType):
    # Get actual start address and byte count to be written.
    startAddress, actualLength = common_util.get_start_address_and_length(bl, length, lengthType, memType, locationType)
    # Create a random file which contains actualLength bytes.
    randomFile = common_util.generate_random_data_file(bl, startAddress, actualLength)
    # Write file to memory
    status, results = bl.write_memory(startAddress, randomFile)
    # Get the location of a given block.
    locationStatus = common_util.block_location(bl, startAddress, actualLength, memType)
    if locationStatus == common_util.kInvalidParameter:
        assert status != bootloader.status.kStatus_Success
    elif locationStatus == common_util.kZeroSizeBlock:
        assert status == bootloader.status.kStatus_Success
    elif locationStatus == common_util.kInvalidMemoryRange:
        assert status == bootloader.status.kStatusMemoryRangeInvalid
    elif locationStatus == common_util.kValidMemoryRange:
        if startAddress % bl.target.programAlignmentSize == 0 or memType == 'ram':
                assert status == bootloader.status.kStatus_Success
        elif startAddress % bl.target.programAlignmentSize > 0:
            assert status == bootloader.status.kStatus_FlashAlignmentError

## Test cases
class TestWriteRam:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('bytesNumber'),[
                            (0),
                            (1),
                            (2),
                            (5),
                            (8)
                            ])
    def test_write_bytes_at_start_of_ram(self, bl, bytesNumber):
        write_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'ram', 'StartOfMemory')

    @pytest.mark.parametrize(('bytesNumber'),[
                            (0),
                            (1),
                            (2),
                            (5),
                            (8)
                            ])
    def test_write_bytes_at_end_of_ram(self, bl, bytesNumber):
        write_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'ram', 'EndOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'),[
                            (0),
                            (1),
                            (2),
                            (3)
                            ])
    def test_write_sectors_at_start_of_ram(self, bl, sectorsNumber):
        write_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'ram', 'StartOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'),[
                            (0),
                            (1),
                            (2),
                            (3)
                            ])
    def test_write_sectors_at_end_of_ram(self, bl, sectorsNumber):
        write_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'ram', 'EndOfMemory')

    def test_write_one_sector_plus_one_byte_at_start_of_ram(self, bl):
        write_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'ram', 'StartOfMemory')

    def test_write_one_sector_plus_one_byte_at_end_of_ram(self, bl):
        write_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'ram', 'EndOfMemory')

    def test_write_half_of_ram(self, bl):
        write_memory_according_to_parameters(bl, 1, 'HalfMemory', 'ram', 'StartOfMemory')

    def test_write_all_of_ram(self, bl):
        write_memory_according_to_parameters(bl, 1, 'AllMemory', 'ram', 'StartOfMemory')

    def test_write_unaligned_ram_address(self, bl):
        common_util.verify_manipulate_unaligned_address_according_to_parameters(bl, 'write-memory', 'ram')


class TestWriteFlashAfterErase:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('bytesNumber'),[
                            (0),
                            (1),
                            (2),
                            (5),
                            (8)
                            ])
    def test_write_bytes_at_start_of_flash(self, bl, bytesNumber):
        write_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'flash', 'StartOfMemory')

    @pytest.mark.parametrize(('bytesNumber'),[
                            (0),
                            (1),
                            (2),
                            (5),
                            (8)
                            ])
    def test_write_bytes_at_end_of_flash(self, bl, bytesNumber):
        write_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'flash', 'EndOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'),[
                            (0),
                            (1),
                            (2),
                            (3)
                            ])
    def test_write_sectors_at_start_of_flash(self, bl, sectorsNumber):
        write_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'flash', 'StartOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'),[
                            (0),
                            (1),
                            (2),
                            (3)
                            ])
    def test_write_sectors_at_end_of_flash(self, bl, sectorsNumber):
        write_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'flash', 'EndOfMemory')

    def test_write_one_sector_plus_one_byte_at_start_of_flash(self, bl):
        write_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'flash', 'StartOfMemory')

    def test_write_one_sector_plus_one_byte_at_end_of_flash(self, bl):
        write_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'flash', 'EndOfMemory')

    def test_write_half_of_flash(self, bl):
        write_memory_according_to_parameters(bl, 1, 'HalfMemory', 'flash', 'StartOfMemory')

    def test_write_all_of_flash(self, bl):
        write_memory_according_to_parameters(bl, 1, 'AllMemory', 'flash', 'StartOfMemory')

    def test_write_unaligned_flash_address(self, bl):
        common_util.verify_manipulate_unaligned_address_according_to_parameters(bl, 'write-memory', 'flash')


class TestCumulativeWrite():
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    def test_cumulative_write(self, bl):
        availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, 'flash')
        # Create a random file which contains 1KB bytes.
        randomFile = common_util.generate_random_data_file(bl, availableRegionStartAddress, 0x400)
        # write file to the specific available flash region
        status, results = bl.write_memory(availableRegionStartAddress, randomFile)
        assert status == bootloader.status.kStatus_Success

        # Create a random file which contains 1KB bytes.
        randomFile = common_util.generate_random_data_file(bl, availableRegionStartAddress, 0x400)
        # attempt to write 0 to 1, expected return value is kStatus_FlashCommandFailure
        status, results = bl.write_memory(availableRegionStartAddress, randomFile)
        assert status == bootloader.status.kStatus_FlashCommandFailure or status == bootloader.status.kStatusMemoryCumulativeWrite


class TestWriteAllAvailableMemory():
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('memType'),[
                            ('flash'),
                            ('ram')
                            ])
    def test_write_all_available_memory(self, bl, memType):
        availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, memType)
        # Create a random file which contains availableRegionSize bytes.
        randomFile = common_util.generate_random_data_file(bl, availableRegionStartAddress, availableRegionSize)
        # write file to the specific available flash region
        status, results = bl.write_memory(availableRegionStartAddress, randomFile)
        assert status == bootloader.status.kStatus_Success

        # Read back the data from Flash
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(availableRegionStartAddress, availableRegionSize, readFile)
        assert status == bootloader.status.kStatus_Success

        # The two data files should be the same.
        assert True == common_util.file_comparison(randomFile, readFile)
