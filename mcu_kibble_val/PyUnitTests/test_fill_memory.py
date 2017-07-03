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
from fsl.bootloader import commands
from common import common_util


## @brief Skip all the tests in this module if the target does not support the fill-memory command.
@pytest.fixture(scope = 'function', autouse = True)
def skipif(bl, request):
    common_util.reset_with_check(bl)
    if (commands.kCommandMask_fillMemory & common_util.get_available_commands(bl)) == 0:
        pytest.skip('%s does not support the fill-memory command.' %(bl.target.targetName))


kFilledValue = 0xFFFFFFFE

## @breif Verify fill-memory command according to given parameters.
# @param: bl
#         length         byte/sector count to be filled with
#         lengthType     'Bytes'                   It means 'length' bytes will be filled into given address with filled value
#                        'Sectors'                 It means 'length' sectors will be filled into given address with filled value
#                        'OneSectorPlusOneByte'    It means 'length' * (1 sector + 1) bytes will be filled info given address with filled value
#                        'HalfMemory'              It means 'length' * half_flash_size bytes will be filled into given address with filled value
#                        'AllMemory'               It means whole memory will be filled with filled value
#         memType        'flash'             represent fill a specified range of flash array
#                        'ram'               represent fill a specified range of ram
#         locationType   'StartOfMemory'     Fill from the start address of given memory (flash or ram)
#                        'EndOfMemory'       Fill from  end address - actual length + 1
def fill_memory_according_to_parameters(bl, length, lengthType, memType, locationType):
    # Get actual start address and byte count to be filled
    startAddress, actualLength = common_util.get_start_address_and_length(bl, length, lengthType, memType, locationType)
    status, results = bl.fill_memory(startAddress, actualLength, kFilledValue, 'word')
    # Get the location of a given block
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

class TestFillFlashAfterErase:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
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
    def test_fill_bytes_at_start_of_flash(self, bl, bytesNumber):
        fill_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'flash', 'StartOfMemory')

    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_fill_bytes_at_end_of_flash(self, bl, bytesNumber):
        fill_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'flash', 'EndOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_fill_sectors_at_start_of_flash(self, bl, sectorsNumber):
        fill_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'flash', 'StartOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_fill_sectors_at_end_of_flash(self, bl, sectorsNumber):
        fill_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'flash', 'EndOfMemory')

    def test_fill_one_sector_plus_one_byte_at_start_of_flash(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'flash', 'StartOfMemory')

    def test_fill_one_sector_plus_one_byte_at_end_of_flash(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'flash', 'EndOfMemory')

    def test_fill_half_of_flash(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'HalfMemory', 'flash', 'StartOfMemory')

    def test_fill_all_of_flash(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'AllMemory', 'flash', 'StartOfMemory')

    def test_fill_unaligned_flash_address(self, bl):
        common_util.verify_manipulate_unaligned_address_according_to_parameters(bl, 'fill-memory', 'flash')

class TestFillRam:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_fill_bytes_at_start_of_ram(self, bl, bytesNumber):
        fill_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'ram', 'StartOfMemory')

    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_fill_bytes_at_end_of_ram(self, bl, bytesNumber):
        fill_memory_according_to_parameters(bl, bytesNumber, 'Bytes', 'ram', 'EndOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_fill_sectors_at_start_of_ram(self, bl, sectorsNumber):
        fill_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'ram', 'StartOfMemory')

    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8)
                            ])
    def test_fill_sectors_at_end_of_ram(self, bl, sectorsNumber):
        fill_memory_according_to_parameters(bl, sectorsNumber, 'Sectors', 'ram', 'EndOfMemory')

    def test_fill_one_sector_plus_one_byte_at_start_of_ram(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'ram', 'StartOfMemory')

    def test_fill_one_sector_plus_one_byte_at_end_of_ram(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'ram', 'EndOfMemory')

    def test_fill_half_of_ram(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'HalfMemory', 'ram', 'StartOfMemory')

    def test_fill_all_of_ram(self, bl):
        fill_memory_according_to_parameters(bl, 1, 'AllMemory', 'ram', 'StartOfMemory')

    def test_fill_unaligned_ram_address(self, bl):
        common_util.verify_manipulate_unaligned_address_according_to_parameters(bl, 'fill-memory', 'ram')

## @breif Verify fill memory with the given pattern format.
def verify_fill_memory_with_pattern_format(bl, startAddress, byteCount, patternFormat, availableRegionSize):
        # Set the pattern as random data.
        if patternFormat == 'byte':
            pattern = random.randint(0,0x000000FF)
        elif patternFormat == 'short':
            pattern = random.randint(0,0x0000FFFF)
        elif patternFormat == 'word':
            pattern = random.randint(0,0xFFFFFFFF)

        # The byteCount in fill-memory should be 4 bytes alignment.
        actualByteCount = common_util.set_data_nbytes_align(byteCount, 4)

        # fill data to specific flash area
        status, results = bl.fill_memory(startAddress, actualByteCount, pattern, patternFormat)
        if actualByteCount > availableRegionSize:
            assert status == bootloader.status.kStatusMemoryRangeInvalid
        elif startAddress % bl.target.programAlignmentSize == 0 or memType == 'ram':
            assert status == bootloader.status.kStatus_Success
            # Read back the filling data and compare with the filling data.
            readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
            status, results = bl.read_memory(startAddress, actualByteCount, readDataFile)
            assert status == bootloader.status.kStatus_Success
            # Validate whether the data written in are the same as the data read out.
            assert common_util.is_fill_memory_correct(readDataFile, actualByteCount, pattern, patternFormat) == True

        elif startAddress % bl.target.programAlignmentSize > 0:
            assert status == bootloader.status.kStatus_FlashAlignmentError

class TestFillFlashWithDifferentFormats:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('patternFormat'),[
                             ('byte'),
                             ('short'),
                             ('word')
                            ])
    def test_fill_all_available_flash_with_pattern_format(self, bl, patternFormat):
        availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, 'flash')
        verify_fill_memory_with_pattern_format(bl, availableRegionStartAddress, availableRegionSize, patternFormat, availableRegionSize)


class TestFillRamWithDifferentFormats:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('patternFormat'),[
                             ('byte'),
                             ('short'),
                             ('word')
                            ])
    def test_fill_all_available_ram_with_pattern_format(self, bl, patternFormat):
        availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, 'ram')
        verify_fill_memory_with_pattern_format(bl, availableRegionStartAddress, availableRegionSize, patternFormat, availableRegionSize)

class TestCumulativeFill:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    def test_cumulative_fill(self, bl):
        availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, 'flash')

        # fill pattern 0x00 to the specific unreserved flash region
        status, results = bl.fill_memory(availableRegionStartAddress, 0x400, 0x00, 'byte')
        assert status == bootloader.status.kStatus_Success

        # attempt to write 0 to 1, expected return value is kStatus_FlashCommandFailure
        status, results = bl.fill_memory(availableRegionStartAddress, 0x400, 0xFF, 'byte')
        assert status == bootloader.status.kStatus_FlashCommandFailure or status == bootloader.status.kStatusMemoryCumulativeWrite
