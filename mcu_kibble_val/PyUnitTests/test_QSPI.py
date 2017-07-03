#! /usr/bin/env python

# Copyright (c) 2015 Freescale Semiconductor, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code mst retain the above copyright notice, this list
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
import time
import binascii

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from PyUnitTests import bltest_config
from common import common_util
from common import user_config_area
from common import flash_config_area
from common import sb_command

sbCmdDict = sb_command.sbCmdDict

# @breif Auto select the QSPI flash type according to the name of kibble_board.
if (bltest_config.projectName.lower() == 'rom_bootloader_tower') or \
   (bltest_config.projectName.lower() == 'rom_bootloader_freedom') or\
   (bltest_config.projectName.lower() == 'tower_bootloader') or\
   (bltest_config.projectName.lower() == 'freedom_bootloader'):
    QSPI_FLASH_TYPE = 'QSPI_FLASH_MX25U3235F'   # On TWR/FRD board
elif (bltest_config.projectName.lower() == 'rom_bootloader_fpga'):
    QSPI_FLASH_TYPE = 'QSPI_FLASH_S25FL129P'    # On FPGA board
else:
    QSPI_FLASH_TYPE = ''

# @breif Get the QSPI flash information according to the qspi flash type.
if QSPI_FLASH_TYPE == 'QSPI_FLASH_S25FL129P':
    # On the FPGA board we always external connect one qspi flash chip with ELEV board.
    qspiFlashTotalSize   = 0x1000000    # 16MB (256 sectors)
    qspiFlashSectorSize  = 0x10000      # 64KB
    qspiFlashPageSize    = 0x100        # 256Bytes
    kQspiConfigBlockFile = 's25fl129_qcb_Dual_mode_low_freq_16MB_qspi.bin'
elif QSPI_FLASH_TYPE == 'QSPI_FLASH_MX25U3235F':
    if bltest_config.projectName.lower() == 'freedom_bootloader' and bltest_config.targetName.lower() == 'kl82z7':
        # On the FRDM-KL82 board it has only one qspi flash chip whose size is 4MB.
        qspiFlashTotalSize   = 0x400000     # 4MB (1024 sectors)
        qspiFlashSectorSize  = 0x1000       # 4KB
        qspiFlashPageSize    = 0x100        # 256Bytes
        kQspiConfigBlockFile = 'mx25u3235f_qcb_Quad_mode_high_freq_4MB_qspi.bin'
    else:
        # On other tower/freedom board it always has two qspi flash chip, each has 4MB memory size
        qspiFlashTotalSize   = 0x800000     # 4MB + 4MB = 8MB (1024 sectors + 1024 sectors = 2048 sectors)
        qspiFlashSectorSize  = 0x1000       # 4KB
        qspiFlashPageSize    = 0x100        # 256Bytes
        kQspiConfigBlockFile = 'mx25u3235f_qcb_Quad_mode_high_freq_8MB_qspi.bin'
else:
    raise ValueError('Do not find the QSPI chip, please check the name of kibble_board!')

# Change according to the bin file required by the hardware chip single/Dual/Quad
# kQspiConfigBlockFile= kQspiConfigBlockDual

qspiFlashStartAddress = 0x68000000
kFillValue = 0xA8


## @breif Skip all the tests in this module if the target does not support QSPI
@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if tgt.isQspiSupported == False:
        pytest.skip('%s does not support QSPI.' %(tgt.targetName))


# --------------------------------------------------------------------------------
# Test cases: write/fill/read/erase the qspi flash without config the qspi flash.
# --------------------------------------------------------------------------------
class TestAccessQSPIWithoutConfigBlockData:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        # Force to reset the target to make sure the qspi is not configured.
        common_util.reset_without_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    def test_write_without_enable_QSPI(self, bl):
        startAddress, actualLength = get_QSPI_flash_address_and_length(1, 'Sectors', 'StartOfMemory')
        # Creat random file that contains actualLength bytes.
        randomFile = common_util.generate_random_data_file(bl, startAddress, actualLength)
        status, results = bl.write_memory(startAddress, randomFile)
        assert status == bootloader.status.kStatus_QspiNotConfigured
 
    def test_fill_without_enable_QSPI(self, bl):
        startAddress, actualLength = get_QSPI_flash_address_and_length(1, 'Sectors', 'StartOfMemory')
        status, results = bl.fill_memory(startAddress, actualLength, kFillValue)
        assert status == bootloader.status.kStatus_QspiNotConfigured
 
    def test_read_without_enable_QSPI(self, bl):
        startAddress, actualLength = get_QSPI_flash_address_and_length(1, 'Sectors', 'StartOfMemory')
        fileName = os.path.abspath(os.path.join(bl.vectorsDir, 'write_data_to_memory.bin'))
        status, results = bl.read_memory(startAddress, actualLength, fileName)
        assert status == bootloader.status.kStatus_QspiNotConfigured
 
    def test_erase_without_enable_QSPI(self, bl):
        startAddress, actualLength = get_QSPI_flash_address_and_length(1, 'Sectors', 'StartOfMemory')
        status, results = bl.flash_erase_region(startAddress, actualLength)
        assert status == bootloader.status.kStatus_QspiNotConfigured
 
    def test_erase_all_without_enable_QSPI(self, bl):
        status, results = bl.flash_erase_all(1)
        assert status == bootloader.status.kStatus_QspiNotConfigured
 
# --------------------------------------------------------------------------------
# Test cases: erase the qspi flash after config the qspi flash.
# --------------------------------------------------------------------------------
class TestQSPIEraseRegion:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        # Write QSPI config block data to RAM and config QSPI module.
        config_QSPI_flash(bl, kQspiConfigBlockFile, 'ram')
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8),
                            (16)
                            ])
    def test_erase_bytes_at_start_of_QSPI_flash(self, bl, bytesNumber):
        erase_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'StartOfMemory')
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (4),
                            (8),
                            (16)
                            ])
    def test_erase_bytes_at_end_of_QSPI_flash(self, bl, bytesNumber):
        erase_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'EndOfMemory')
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (1),
                            (2)
                            ])
    def test_erase_sectors_at_start_of_QSPI_flash(self, bl, sectorsNumber):
        erase_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'StartOfMemory')
 
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (1),
                            (2)
                            ])
    def test_erase_sectors_at_end_of_QSPI_flash(self, bl, sectorsNumber):
        erase_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'EndOfMemory')
 
    def test_erase_one_sector_plus_one_byte_at_start_of_QSPI_flash(self, bl):
        erase_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'StartOfMemory')
 
    def test_erase_one_sector_plus_one_byte_at_end_of_QSPI_flash(self, bl):
        erase_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'EndOfMemory')
 
    # Start address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_erase_start_address_out_of_range(self, bl):
        erase_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'StartAddressOutOfRange')
 
    # End address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_erase_end_address_out_of_range(self, bl):
        erase_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'EndAddressOutOfRange')
 
# --------------------------------------------------------------------------------
# Test cases: write the qspi flash after config the qspi flash.
# --------------------------------------------------------------------------------
class TestWriteQSPIFlash:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        # Write QSPI config block data to RAM and config QSPI module.
        config_QSPI_flash(bl, kQspiConfigBlockFile, 'ram')
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (3),
                            (8),
                            (16)
                            ])
    def test_write_bytes_at_start_of_QSPI_flash(self, bl, bytesNumber):
        write_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'StartOfMemory')
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (3),
                            (8),
                            (16)
                            ])
    def test_write_bytes_at_end_of_QSPI_flash(self, bl, bytesNumber):
        write_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'EndOfMemory')
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (1),
                            (2)
                            ])
    def test_write_sectors_at_start_of_QSPI_flash(self, bl, sectorsNumber):
        write_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'StartOfMemory')
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (1),
                            (2)
                            ])
    def test_write_sectors_at_end_of_QSPI_flash(self, bl, sectorsNumber):
        write_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'EndOfMemory')
 
    def test_write_one_sector_plus_one_byte_at_start_of_QSPI_flash(self, bl):
        write_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'StartOfMemory')
 
    def test_write_one_sector_plus_one_byte_at_end_of_QSPI_flash(self, bl):
        write_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'EndOfMemory')
 
    # Start address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_write_start_address_out_of_QSPI_memory_range(self, bl):
        write_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'StartAddressOutOfRange')
 
    # End address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_write_end_address_out_of_QSPI_memory_range(self, bl):
        write_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'EndAddressOutOfRange')
 
    def test_write_unaligned_QSPI_flash_address(self, bl):
        # Generate a file that contains 1024 bytes random data
        randomFilePath = common_util.generate_random_data_file(bl, qspiFlashStartAddress, 1024)
 
        # The start address must be page size aligned when program the qspi flash.
        alignBase = qspiFlashPageSize
        for i in range(1, alignBase):
            status, results = bl.write_memory(qspiFlashStartAddress + i, randomFilePath)
            assert status == bootloader.status.kStatus_QspiFlashAlignmentError
 
    def test_cumulative_write_QSPI_flash(self, bl):
        startAddress = qspiFlashStartAddress
        actualLength = qspiFlashPageSize
 
        # Erase the qspi flash
        status, results = bl.flash_erase_region(startAddress, actualLength)
        assert status == bootloader.status.kStatus_Success
 
        # Generate a file that contains actualLength bytes random data
        randomFilePath = common_util.generate_random_data_file(bl, startAddress, actualLength)
 
        # Write qspi flash after erase should be success
        status, results = bl.write_memory(startAddress, randomFilePath)
        assert status == bootloader.status.kStatus_Success
 
        # Generate a file that contains actualLength bytes random data
        randomFilePath = common_util.generate_random_data_file(bl, startAddress, actualLength)
 
        # Write qspi flash without erase should be fail
        status, results = bl.write_memory(startAddress, randomFilePath)
        assert status == bootloader.status.kStatus_QspiFlashCommandFailure
 
# --------------------------------------------------------------------------------
# Test cases: fill the qspi flash after config the qspi flash.
# --------------------------------------------------------------------------------
class TestFillQSPIFlash:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        # Write QSPI config block data to RAM and config QSPI module.
        config_QSPI_flash(bl, kQspiConfigBlockFile, 'ram')
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (3),
                            (8),
                            (16)
                            ])
    def test_fill_bytes_at_start_of_QSPI_flash(self, bl, bytesNumber):
        fill_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'StartOfMemory')
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (3),
                            (8),
                            (16)
                            ])
    def test_fill_bytes_at_end_of_QSPI_flash(self, bl, bytesNumber):
        fill_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'EndOfMemory')
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (1),
                            (2)
                            ])
    def test_fill_sectors_at_start_of_QSPI_flash(self, bl, sectorsNumber):
        fill_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'StartOfMemory')
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (1),
                            (2)
                            ])
    def test_fill_sectors_at_end_of_QSPI_flash(self, bl, sectorsNumber):
        fill_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'EndOfMemory')
 
    def test_fill_one_sector_plus_one_byte_at_start_of_QSPI_flash(self, bl):
        fill_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'StartOfMemory')
 
    def test_fill_one_sector_plus_one_byte_at_end_of_QSPI_flash(self, bl):
        fill_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'EndOfMemory')
 
    # Start address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_fill_start_address_out_of_range(self, bl):
        fill_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'StartAddressOutOfRange')
 
    # End address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_fill_end_address_out_of_range(self, bl):
        fill_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'EndAddressOutOfRange')
 
    def test_fill_unaligned_QSPI_flash_address(self, bl):
        # The start address must be page size aligned when program the qspi flash.
        alignBase = qspiFlashPageSize
        actualLength = qspiFlashPageSize
        for i in range(1, alignBase):
            status, results = bl.fill_memory(qspiFlashStartAddress + i, actualLength, kFillValue)
            assert status == bootloader.status.kStatus_QspiFlashAlignmentError
 
    def test_fill_unaligned_QSPI_flash_length(self, bl):
        alignBase = qspiFlashPageSize
        for i in range(1, alignBase):
            # Erase the qspi flash
            status, results = bl.flash_erase_region(qspiFlashStartAddress, i)
            assert status == bootloader.status.kStatus_Success
            # Fill unaligned length data to qspi flash
            status, results = bl.fill_memory(qspiFlashStartAddress, i, kFillValue)
            # It always returns success.
            assert status == bootloader.status.kStatus_Success
 
    def test_cumulative_fill_QSPI_flash(self, bl):
        startAddress = qspiFlashStartAddress
        actualLength = qspiFlashPageSize
 
        # Erase the qspi flash
        status, results = bl.flash_erase_region(startAddress, actualLength)
        assert status == bootloader.status.kStatus_Success
 
        # fill qspi flash after erase should be success
        status, results = bl.fill_memory(startAddress, actualLength, kFillValue, 'word')
        assert status == bootloader.status.kStatus_Success
 
        # fill qspi flash without erase should be fail
        status, results = bl.fill_memory(startAddress, actualLength, kFillValue, 'byte')
        assert status == bootloader.status.kStatus_QspiFlashCommandFailure
 
# --------------------------------------------------------------------------------
# Test cases: read the qspi flash after config the qspi flash.
# --------------------------------------------------------------------------------
class TestReadQSPIFlash:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        # Write QSPI config block data to RAM and config QSPI module.
        config_QSPI_flash(bl, kQspiConfigBlockFile, 'ram')
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (3),
                            (8),
                            (16)
                            ])
    def test_read_bytes_at_start_of_QSPI_flash(self, bl, bytesNumber):
        read_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'StartOfMemory')
 
    @pytest.mark.parametrize(('bytesNumber'), [
                            (0),
                            (1),
                            (2),
                            (3),
                            (8),
                            (16)
                            ])
    def test_read_bytes_at_end_of_QSPI_flash(self, bl, bytesNumber):
        read_QSPI_flash_according_to_parameters(bl, bytesNumber, 'Bytes', 'EndOfMemory')
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2)
                            ])
    def test_read_sectors_at_start_of_QSPI_flash(self, bl, sectorsNumber):
        read_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'StartOfMemory')
 
    @pytest.mark.parametrize(('sectorsNumber'), [
                            (0),
                            (1),
                            (2)
                            ])
    def test_read_sectors_at_end_of_QSPI_flash(self, bl, sectorsNumber):
        read_QSPI_flash_according_to_parameters(bl, sectorsNumber, 'Sectors', 'EndOfMemory')
 
    def test_read_one_sector_plus_one_byte_at_start_of_QSPI_flash(self, bl):
        read_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'StartOfMemory')
 
    def test_read_one_sector_plus_one_byte_at_end_of_QSPI_flash(self, bl):
        read_QSPI_flash_according_to_parameters(bl, 1, 'OneSectorPlusOneByte', 'EndOfMemory')
 
    # Start address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_read_QSPI_start_address_out_of_range(self, bl):
        read_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'StartAddressOutOfRange')
 
    # End address is out of qspi flash memory range and return kStatusMemoryRangeInvalid.
    def test_read_QSPI_end_address_out_of_range(self, bl):
        read_QSPI_flash_according_to_parameters(bl, 1024, 'Bytes', 'EndAddressOutOfRange')
 
 
# ------------------------------------------------------------------------------------------------
# Test cases: test the behavior of accessing the qspi flash with resetting target after config qspi.
# ------------------------------------------------------------------------------------------------
class TestAccessQSPIFlashAfterReset:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    @pytest.mark.parametrize(('qcbLocation'), [
                            ('ram'),
                            ('flash')
                            ])
    def test_access_QSPI_with_reset_after_QSPI_enabled(self, bl, qcbLocation):
        # 1. Write QSPI config block data to ram/flash and config QSPI module.
        config_QSPI_flash(bl, kQspiConfigBlockFile, qcbLocation)
 
        # 2. Access qspi flash should be successful after configure qspi
        startAddress = qspiFlashStartAddress
        length = qspiFlashPageSize
        # Erase
        status, results = bl.flash_erase_region(startAddress, length)
        assert status == bootloader.status.kStatus_Success
        # Write
        randomFilePath = common_util.generate_random_data_file(bl, startAddress, length)
        status, results = bl.write_memory(startAddress, randomFilePath)
        assert status == bootloader.status.kStatus_Success
        # Fill
        status, results = bl.fill_memory(startAddress, length, kFillValue)
        assert status == bootloader.status.kStatus_QspiFlashCommandFailure
        # Read
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startAddress, length, readFile)
        assert status == bootloader.status.kStatus_Success
 
        # 3. Access qspi flash should fail after rest.
        # Reset
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        startAddress = qspiFlashStartAddress
        length = qspiFlashPageSize
        # Erase
        status, results = bl.flash_erase_region(startAddress, length)
        assert status == bootloader.status.kStatus_QspiNotConfigured
        # Write
        randomFilePath = common_util.generate_random_data_file(bl, startAddress, length)
        status, results = bl.write_memory(startAddress, randomFilePath)
        assert status == bootloader.status.kStatus_QspiNotConfigured
        # Fill
        status, results = bl.fill_memory(startAddress, length, kFillValue)
        assert status == bootloader.status.kStatus_QspiNotConfigured
        # Read
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startAddress, length, readFile)
        assert status == bootloader.status.kStatus_QspiNotConfigured
 
 
# ------------------------------------------------------------------------------------------------
# Test cases: let ROM code configure qspi automatically.
#      case one: QCB in flash/ram
#          1. write qspi configure block data to flash/ram.
#          2. set the qspi configure block pointer in the BCA.
#          3. set FOPT to let MCU boot from ROM, configure qspi and enter into bootloader mode.
#          4. reset target.
#          5. qspi can be accessed even reset the target.
#
#      case two: QCB in qspi flash start address
#          1. make sure QCB is located in qspi flash start address (0x6800_0000).
#          2. set FOPT to let MCU boot from ROM, configure qspi and enter into bootloader mode.
#          3. reset target.
#          4. qspi can be accessed even reset the target.
# ------------------------------------------------------------------------------------------------
class TestAutoConfigQSPI:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        # Force to reset the target
        common_util.reset_without_check(bl)
        def teardown():
            # Set flash as unsecure state
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)
 
    @pytest.mark.parametrize(('qcbLocation'), [
                            # ROM code does not consider the condition when QCB locates in ram becuase ram data will be lost after reset.
                            #('ram'),
 
                            # QCB cannot be located in flash start address as it is used to place the app's vector table.
                            # Let QCB locate at the end of the flash address for the boundary value testing.
                            ('flash')
                            ])
    def test_config_QCB_at_internal_memory(self, bl, qcbLocation):
        # 1. Erase the whole flash
        status, results = bl.flash_erase_all()
        assert status == bootloader.status.kStatus_Success
 
        # 2. get the available address to place the qspi config block data and set qspiConfigBlockPointer to BCA
        qspiConfigBlockPointer = config_QSPI_flash(bl, kQspiConfigBlockFile, qcbLocation)
 
        bcaFilePath = user_config_area.create_BCA_data_file(bl, qspiConfigBlockPointer = qspiConfigBlockPointer)
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
 
        # 3. Set FOPT to let MCU boot from ROM, configure qspi and enter into bootloader mode.
        # For the flash-resident bootloader on the ROM tape-out chip, the FOPT is already 0xBD,
        # so no need to configure FOPT here, what's more it will get Flash_Command_Failure.
        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            flashConfigDataFilePath = flash_config_area.generate_flash_config_data(flashOptionRegister = 0xBD) # 0xBD/0xBF is OK
            status, results = bl.write_memory(0x400, flashConfigDataFilePath)
            assert status == bootloader.status.kStatus_Success
        else:
            pass
 
        # 4. Reset target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
 
        # 5. After reset, ROM code will configure qspi automatically, so accessing qspi flash should be successful
        startAddress = qspiFlashStartAddress
        length = qspiFlashPageSize
        # Erase
        status, results = bl.flash_erase_region(startAddress, length)
        assert status == bootloader.status.kStatus_Success
        # Write
        randomFilePath = common_util.generate_random_data_file(bl, startAddress, length)
        status, results = bl.write_memory(startAddress, randomFilePath)
        assert status == bootloader.status.kStatus_Success
        # Fill
        status, results = bl.fill_memory(startAddress, length, kFillValue)
        assert status == bootloader.status.kStatus_QspiFlashCommandFailure
        # Read
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startAddress, length, readFile)
        assert status == bootloader.status.kStatus_Success
 
    # This case can pass on the ROM tape out board, while fail on the fpga board.
    # on fpga board, it seeems we must press the SW3/SW5 to reset in the step 4, so that this case can pass, maybe it's a fpga board issue.
    def test_config_QCB_at_QSPI_start_address(self, bl):
        # 1. Make sure QCB is located in qspi flash start address
        # Write QSPI config block data to RAM and config QSPI module.
        config_QSPI_flash(bl, kQspiConfigBlockFile, 'ram')
        # Erase qspi flash.
        status, results = bl.flash_erase_region(qspiFlashStartAddress, qspiFlashSectorSize)
        assert status == bootloader.status.kStatus_Success
        # Write QCB to qspi flash start address
        fullFileName = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))
        status, results = bl.write_memory(qspiFlashStartAddress, fullFileName)
        assert status == bootloader.status.kStatus_Success
 
        # 2. Reset target and access the qspi flash will fail for ROM.
        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            # Reset
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)
            startAddress = qspiFlashStartAddress
            length = qspiFlashPageSize
            # Erase
            status, results = bl.flash_erase_region(startAddress, length)
            assert status == bootloader.status.kStatus_QspiNotConfigured
            # Write
            randomFilePath = common_util.generate_random_data_file(bl, startAddress, length)
            status, results = bl.write_memory(startAddress, randomFilePath)
            assert status == bootloader.status.kStatus_QspiNotConfigured
            # Fill
            status, results = bl.fill_memory(startAddress, length, kFillValue)
            assert status == bootloader.status.kStatus_QspiNotConfigured
            # Read
            readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
            status, results = bl.read_memory(startAddress, length, readFile)
            assert status == bootloader.status.kStatus_QspiNotConfigured
     
            # 3. Set FOPT to let MCU boot from ROM, configure qspi and enter into bootloader mode.
            # For the flash-resident bootloader on the ROM tape-out chip, the FOPT is already 0xBD.
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
            flashConfigDataFilePath = flash_config_area.generate_flash_config_data(flashOptionRegister = 0xBD) # 0xBD/0xBF is OK
            status, results = bl.write_memory(0x400, flashConfigDataFilePath)
            assert status == bootloader.status.kStatus_Success
        else:
            pass
        # 4. After reset, ROM code will configure qspi automatically, so accessing qspi flash should be successful
        # Reset. It seeems we must press the SW3/SW5 to reset the target on fpga board, so that this case can pass, maybe it's a fpga board issue.
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        startAddress = qspiFlashStartAddress
        length = qspiFlashPageSize
        # Erase
        status, results = bl.flash_erase_region(startAddress, length)
        assert status == bootloader.status.kStatus_Success
        # Write
        randomFilePath = common_util.generate_random_data_file(bl, startAddress, length)
        status, results = bl.write_memory(startAddress, randomFilePath)
        assert status == bootloader.status.kStatus_Success
        # Fill
        status, results = bl.fill_memory(startAddress, length, kFillValue)
        assert status == bootloader.status.kStatus_QspiFlashCommandFailure
        # Read
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startAddress, length, readFile)
        assert status == bootloader.status.kStatus_Success

# --------------------------------------------------------------------------------
# Test cases: Erase/Write/Fill/Read half of the qspi flash.
#             Erase/Write/Fill/Read all of the qspi flash.
# Note: It will take a long time to finish the following cases, so if you just
#       want to have a sanity check with the qspi module, you can comment the
#       following test cases.
# --------------------------------------------------------------------------------
class TestAccessHalfAndAllQSPIFlash:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        # Write QSPI config block data to RAM and config QSPI module.
        config_QSPI_flash(bl, kQspiConfigBlockFile, 'ram')
        def teardown():
            pass
        request.addfinalizer(teardown)

    # Access half of the qspi flash
    def test_erase_half_of_QSPI_flash(self, bl):
        erase_QSPI_flash_according_to_parameters(bl, 1, 'HalfMemory', 'StartOfMemory')

    def test_write_half_of_QSPI_flash(self, bl):
        write_QSPI_flash_according_to_parameters(bl, 1, 'HalfMemory', 'StartOfMemory')

    def test_fill_half_of_QSPI_flash(self, bl):
        fill_QSPI_flash_according_to_parameters(bl, 1, 'HalfMemory', 'StartOfMemory')

    def test_read_half_of_QSPI_flash(self, bl):
        read_QSPI_flash_according_to_parameters(bl, 1, 'HalfMemory', 'StartOfMemory')

    # Access the whole qspi flash
    def test_erase_all_of_QSPI_flash(self, bl):
        # Use flash-erase-region to erase the whole qspi flash.
        erase_QSPI_flash_according_to_parameters(bl, 1, 'AllMemory', 'StartOfMemory')

    def test_flash_erase_all_QSPI_flash(self, bl):
        # Use flash-erase-all to erase the whole qspi flash.
        status, results = bl.flash_erase_all(1)
        assert status == bootloader.status.kStatus_Success

    def test_write_all_of_QSPI_flash(self, bl):
        write_QSPI_flash_according_to_parameters(bl, 1, 'AllMemory', 'StartOfMemory')

    def test_fill_all_of_QSPI_flash(self, bl):
        fill_QSPI_flash_according_to_parameters(bl, 1, 'AllMemory', 'StartOfMemory')

    def test_read_all_of_QSPI_flash(self, bl):
        read_QSPI_flash_according_to_parameters(bl, 1, 'AllMemory', 'StartOfMemory')

#===============================================================================
# Test cases: Enable qspi with sb file.
#             Erase all qspi with sb file.
#===============================================================================
class TestQSPIWithSBCommand:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        # Force to reset target
        common_util.reset_without_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('qcbLocation'),[
                             # Currently QCB is not supported to locate at flash start address
                             (0),
                             # It failed when tested on TWR-K80F180 TO 1.1, not sure why it fails when locate at 0x1FFFFFFF
                             #(0x1FFFFFFF),
                             (0x20000000)
                            ])
    def test_enable_QSPI_with_sb_file(self, bl, qcbLocation):
        # Operate qspi flash will fail before enable qspi
        startAddress, actualLength = get_QSPI_flash_address_and_length(1, 'Sectors', 'StartOfMemory')
        # Creat random file that contains actualLength bytes.
        randomFile = common_util.generate_random_data_file(bl, startAddress, actualLength)
        status, results = bl.write_memory(startAddress, randomFile)
        assert status == bootloader.status.kStatus_QspiNotConfigured

        # Get qspi block data
        sbCmdDict['enableQspi'].qspiConfigBlock = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))
        # Get the location of the qspi block data
        sbCmdDict['enableQspi'].qcbLocation = qcbLocation
        # Generate sb file
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'internalFlashEraseAll', 'enableQspi')
        status, results = bl.receive_sb_file(sbFilePath)
        if qcbLocation == 0:
            if bl.target.bootloaderType == bootsources.kBootFlash_ExecuteFlash:
                # The memory address 0x0 is in the reserved region for flash-resident bootloader.
                assert status == bootloader.status.kStatusMemoryRangeInvalid
            else:
                # Cannot enable QSPI at 0x0 even it is available.
                assert status == bootloader.status.kStatus_InvalidArgument

            # Operate qspi flash will fail
            status, results = bl.flash_erase_region(startAddress, actualLength)
            assert status == bootloader.status.kStatus_QspiNotConfigured
        else:
            assert status == bootloader.status.kStatus_Success
            # Operate qspi flash will be successful after enable qspi
            status, results = bl.flash_erase_region(startAddress, actualLength)
            assert status == bootloader.status.kStatus_Success

    def test_erase_all_QSPI_with_sb_file(self, bl):
        # Get qspi block data
        sbCmdDict['enableQspi'].qspiConfigBlock = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))
        # Get the location of the qspi block data
        sbCmdDict['enableQspi'].qcbLocation = 0x20000000
        # Generate sb file
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'enableQspi', 'qspiFlashEraseAll')
        print sbFilePath
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success


# -----------------------------------------------------------------------------------------------
# Functions:
#     1. get_QSPI_flash_address_and_length(length, lengthType, locationType)
#     2. config_QSPI_flash(bl, qcbFileName, qcbResident)
#     3. block_location(startAddress, length)
#     4. erase_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType)
#     5. write_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType)
#     6. fill_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType)
#     7. read_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType)
# -----------------------------------------------------------------------------------------------
#         length         byte/sector
#         lengthType     'Bytes'                   'length' bytes
#                        'Sectors'                 'length' sectors
#                        'OneSectorPlusOneByte'    'length' * (1 sector + 1) bytes
#                        'HalfMemory'              'length' * half_flash_size bytes
#                        'AllMemory'               whole memory
#         locationType   'StartOfMemory'           startAddr = start address
#                        'EndOfMemory'             startAddr = end address - actual length + 1
#                        'StartAddressOutOfRange'
#                        'EndAddressOutOfRange'
def get_QSPI_flash_address_and_length(length, lengthType, locationType):
    memorySize = qspiFlashTotalSize

    if lengthType == 'Bytes':
        actualLength = length
    elif lengthType == 'Sectors':
        actualLength = length * qspiFlashSectorSize
    elif lengthType == 'OneSectorPlusOneByte':
        actualLength = length * (1 + qspiFlashSectorSize)
    elif lengthType == 'HalfMemory':
        actualLength = memorySize / 2
    elif lengthType == 'AllMemory':
        actualLength = memorySize
    else:
        raise ValueError('No such lengthType.')

    if locationType == 'StartOfMemory':
        startAddress = qspiFlashStartAddress
    elif locationType == 'EndOfMemory':
        startAddress = qspiFlashStartAddress + memorySize - actualLength
    elif locationType == 'StartAddressOutOfRange':
        startAddress = qspiFlashStartAddress + memorySize
    elif locationType == 'EndAddressOutOfRange':
        startAddress = qspiFlashStartAddress + memorySize - 1
    else:
        raise ValueError('No such locationType.')

    return startAddress, actualLength

## @brief Configure qspi flash
def config_QSPI_flash(bl, qcbFileName, qcbResident):
    # get the file path
    qcbFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', qcbFileName))
    # Get available memory address to place the QCB
    availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, qcbResident)
    qcbLocation = availableRegionStartAddress

    if qcbResident == 'ram':
        pass
    elif qcbResident == 'flash':
        if availableRegionStartAddress == 0:
            # QCB cannot be located in flash start address as it is used to place the app's vector table.
            # Let QCB locate at the end of the flash address for the boundary value testing.
            qcbLocation = qcbLocation + common_util.get_memory_total_size(bl, 'flash') - os.path.getsize(qcbFilePath)
        else:
            pass

        # Erase 1 sector flash before program
        flashSectorSize = common_util.get_flash_sector_size(bl)
        startAddressSectorAlign = common_util.flash_align_down(qcbLocation, flashSectorSize)
        status, results = bl.flash_erase_region(startAddressSectorAlign, flashSectorSize)
        assert status == bootloader.status.kStatus_Success

    # write qspi config block data to ram
    status, results = bl.write_memory(qcbLocation, qcbFilePath)
    assert status == bootloader.status.kStatus_Success
    # execute configure-quadspi command
    status, results = bl.configure_memory(1, qcbLocation)
    assert status == bootloader.status.kStatus_Success
    return qcbLocation

## @breif This function is to estimate where the block data are located in.
def block_location(startAddress, length):
    endAddress = startAddress + length - 1

    if length == 0:
        locationStatus = common_util.kZeroSizeBlock
    elif (startAddress >  qspiFlashStartAddress + qspiFlashTotalSize - 1) or \
       (endAddress   >  qspiFlashStartAddress + qspiFlashTotalSize - 1) or \
       (startAddress < qspiFlashStartAddress):
        locationStatus = common_util.kInvalidMemoryRange
    else:
        locationStatus = common_util.kValidMemoryRange
    return locationStatus

## @breif Verify erase qspi flash according to given parameters.
def erase_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType):
    # Get actual start address and byte count to be erased.
    startAddress, actualLength = get_QSPI_flash_address_and_length(length, lengthType, locationType)
    status, results = bl.flash_erase_region(startAddress, actualLength)
    if block_location(startAddress, actualLength) == common_util.kInvalidMemoryRange:
        assert status == bootloader.status.kStatusMemoryRangeInvalid
    else:
        # It always returns success for every kind of input parameters, because ROM code will deal with it internally.
        assert status == bootloader.status.kStatus_Success

## brief Verify QSPI flash write
def write_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType):
    startAddress, actualLength = get_QSPI_flash_address_and_length(length, lengthType, locationType)
    # Erase the qspi flash before program it.
    eraseStatus, results = bl.flash_erase_region(startAddress, actualLength)
    # Generate a file contains random contents with actualLength bytes
    randomFilePath = common_util.generate_random_data_file(bl, startAddress, actualLength)
    # Write memory
    writeStatus, results = bl.write_memory(startAddress, randomFilePath)
    # Get the location of the block data
    location = block_location(startAddress, actualLength)

    if location == common_util.kZeroSizeBlock:
        assert eraseStatus == bootloader.status.kStatus_Success
        assert writeStatus == bootloader.status.kStatus_Success
    elif location == common_util.kInvalidMemoryRange:
        assert eraseStatus == bootloader.status.kStatusMemoryRangeInvalid
        assert writeStatus == bootloader.status.kStatusMemoryRangeInvalid
    elif location == common_util.kValidMemoryRange:
        assert eraseStatus == bootloader.status.kStatus_Success
        # The start address must be page size aligned when program the qspi flash.
        if (startAddress % qspiFlashPageSize) == 0:
            assert writeStatus == bootloader.status.kStatus_Success

            # read back the data
            readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
            read_status, results = bl.read_memory(startAddress, actualLength, readFile)
            assert read_status == bootloader.status.kStatus_Success

            # The two data files should be the same.
            assert True == common_util.file_comparison(randomFilePath, readFile)
        elif (startAddress % qspiFlashPageSize) > 0:
            assert writeStatus == bootloader.status.kStatus_QspiFlashAlignmentError

## brief Verify QSPI flash fill
def fill_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType):
    startAddress, actualLength = get_QSPI_flash_address_and_length(length, lengthType, locationType)
    # Erase the qspi flash before program it.
    eraseStatus, results = bl.flash_erase_region(startAddress, actualLength)
    # Fill memory
    fillStatus, results = bl.fill_memory(startAddress, actualLength, kFillValue)
    # Get the location of the block data
    location = block_location(startAddress, actualLength)

    if location == common_util.kZeroSizeBlock:
        assert eraseStatus == bootloader.status.kStatus_Success
        assert fillStatus == bootloader.status.kStatus_Success
    elif location == common_util.kInvalidMemoryRange:
        assert eraseStatus == bootloader.status.kStatusMemoryRangeInvalid
        assert fillStatus == bootloader.status.kStatusMemoryRangeInvalid
    elif location == common_util.kValidMemoryRange:
        assert eraseStatus == bootloader.status.kStatus_Success
        # The start address must be page size aligned when program the qspi flash.
        if (startAddress % qspiFlashPageSize) == 0:
            assert fillStatus == bootloader.status.kStatus_Success
        elif (startAddress % qspiFlashPageSize) > 0:
            assert fillStatus == bootloader.status.kStatus_QspiFlashAlignmentError

## brief Verify QSPI flash read
def read_QSPI_flash_according_to_parameters(bl, length, lengthType, locationType):
    startAddress, actualLength = get_QSPI_flash_address_and_length(length, lengthType, locationType)
    readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
    status, results = bl.read_memory(startAddress, actualLength, readFile)

    if block_location(startAddress, actualLength) == common_util.kInvalidMemoryRange:
        assert status == bootloader.status.kStatusMemoryRangeInvalid
    else:
        assert status == bootloader.status.kStatus_Success




# def modify_bin_file_contents(binFileName, position, stringValue):
#     fileObj = open(binFileName, 'r+')
#     fileObj.seek(position)
#     fileObj.write(binascii.a2b_hex(stringValue))
#     fileObj.close()
#     return binFileName
#
# def get_QSPI_config_block_data(bl, QSPI_mode, sclk_freq):
#     if QSPI_mode == 'single':
#         if sclk_freq == 'low_freq':
#             stringValue = '00'
#         elif sclk_freq == 'mid_freq':
#             stringValue = '01'
#         elif sclk_freq == 'high_freq':
#             stringValue = '02'
#         fileName = os.path.join(bl.vectorsDir, 'QSPI', 'QSPI_config_data_single_pad_low_freq.bin')
#         modifiedFilePath = modify_bin_file_contents(fileName, 0x44, stringValue)
#     elif QSPI_mode == 'dual':
#         if sclk_freq == 'low_freq':
#             stringValue = '00'
#         elif sclk_freq == 'mid_freq':
#             stringValue = '01'
#         elif sclk_freq == 'high_freq':
#             stringValue = '02'
#         fileName = os.path.join(bl.vectorsDir, 'QSPI', 'QSPI_config_data_dual_pad_low_freq.bin')
#         modifiedFilePath = modify_bin_file_contents(fileName, 0x44, stringValue)
#     elif QSPI_mode == 'quad':
#         if sclk_freq == 'low_freq':
#             stringValue = '00'
#         elif sclk_freq == 'mid_freq':
#             stringValue = '01'
#         elif sclk_freq == 'high_freq':
#             stringValue = '02'
#         fileName = os.path.join(bl.vectorsDir, 'QSPI', 'QSPI_config_data_quad_pad_low_freq.bin')
#         modifiedFilePath = modify_bin_file_contents(fileName, 0x44, stringValue)
#     return modifiedFilePath

