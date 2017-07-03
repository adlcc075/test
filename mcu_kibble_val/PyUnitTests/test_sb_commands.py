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

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from fsl.bootloader import encryptiontypes
from fsl.bootloader import commands
from common import common_util
from common import sb_command


sb_commandDictionay = sb_command.sbCmdDict
# Filled value for simple sb file
kFilledValue = 0xFFFFFFFE


## @brief Skip all the tests in this module if the target does not support the receive-sb-file command.
@pytest.fixture(scope = 'function', autouse = True)
def skipif(bl, request):
    common_util.reset_with_check(bl)
    if (commands.kCommandMask_receiveSbFile & common_util.get_available_commands(bl)) == 0:
        pytest.skip('%s does not support the receive-sb-file command.' %(bl.target.targetName))

class TestSbCommandSpecialCases:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    # KBL-1031, KBL-1272, KBL-1388
    @pytest.mark.parametrize(('data_length'), [
                            # build when addr + sizeof(binFile) > 0xFFFFFFFF, otherwise, won't build
                            (0),        # won't build, should return kStatusRomLdrIdNotFound
                            (0x100),    # won't build, should return kStatusRomLdrIdNotFound
                            (0x1000)    # will build, should return kStatusMemoryRangeInvalid
                            ])
    def test_sb_load_bytes_to_zero_size_space(self, bl, data_length):
        # data size is not zero while start address is equal to end address (return kStatusRomLdrIdNotFound)
        sb_commandDictionay['writeMemory'].cumulativeWrite = False
        sb_commandDictionay['writeMemory'].length = data_length
        sb_commandDictionay['writeMemory'].dataType = 'file_bin'
        sb_commandDictionay['writeMemory'].startAddress = 0xFFFFF000
        sb_commandDictionay['writeMemory'].endAddress = sb_commandDictionay['writeMemory'].startAddress
        sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, sb_commandDictionay['writeMemory'].startAddress, sb_commandDictionay['writeMemory'].length)
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatusRomLdrIdNotFound

    def test_sb_load_nothing_to_nonzero_size_space(self, bl):
        # data size is zero while start address is not equal to end address (return kStatusRomLdrChecksum)
        sb_commandDictionay['writeMemory'].cumulativeWrite = False
        sb_commandDictionay['writeMemory'].length = 0
        sb_commandDictionay['writeMemory'].dataType = 'file_bin'
        sb_commandDictionay['writeMemory'].startAddress = 0x1000
        sb_commandDictionay['writeMemory'].endAddress = 0x2000
        sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, sb_commandDictionay['writeMemory'].startAddress, sb_commandDictionay['writeMemory'].length)
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatusRomLdrIdNotFound

    def test_sb_load_nothing_to_zero_size_space(self, bl):
        # data size is zero while start address is equal to end address (return kStatusRomLdrChecksum)
        sb_commandDictionay['writeMemory'].cumulativeWrite = False
        sb_commandDictionay['writeMemory'].length = 0
        sb_commandDictionay['writeMemory'].dataType = 'file_bin'
        sb_commandDictionay['writeMemory'].startAddress = 0xFFFFFFFF
        sb_commandDictionay['writeMemory'].endAddress = sb_commandDictionay['writeMemory'].startAddress
        sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, sb_commandDictionay['writeMemory'].startAddress, sb_commandDictionay['writeMemory'].length)
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        # if load zero length data to zero length space memory, it will return kStatusRomLdrIdNotFound
        assert status == bootloader.status.kStatusRomLdrIdNotFound

class TestSbCommandWriteMemory:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('memType'), [
                             ('flash'),
                             ('ram')
                             ])
    def test_sb_write_all_available_region(self, bl, memType):
        sb_commandDictionay['writeMemory'].cumulativeWrite = False

        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        sb_commandDictionay['writeMemory'].length = length
        sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, startAddress, length)
        sb_commandDictionay['writeMemory'].dataType = 'file_bin'
        sb_commandDictionay['writeMemory'].startAddress = startAddress
        sb_commandDictionay['writeMemory'].endAddress = startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success

        # Read back the data from memory
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startAddress, length, readFile)
        assert status == bootloader.status.kStatus_Success

        # The two data files should be the same.
        with open(sb_commandDictionay['writeMemory'].data, 'rb') as fileObj1:
            data1 = fileObj1.read()
            fileObj1.close()
        with open(readFile, 'rb') as fileObj2:
            data2 = fileObj2.read()
            fileObj2.close()

        assert data1 == data2

    @pytest.mark.parametrize(('memType'), [
                             ('flash'),
                             ('ram')
                             ])
    def test_sb_write_all_reserved_region(self, bl, memType):
        sb_commandDictionay['writeMemory'].cumulativeWrite = False

        startAddress, endAddress, length = common_util.get_reserved_memory_region(bl, memType)
        if length == 0:
            pass    # ROM has no flash reserved region.
        else:
            sb_commandDictionay['writeMemory'].length = length
            sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, startAddress, length)
            sb_commandDictionay['writeMemory'].dataType = 'file_bin'
            sb_commandDictionay['writeMemory'].startAddress = startAddress
            sb_commandDictionay['writeMemory'].endAddress = startAddress + length
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatusMemoryRangeInvalid

    @pytest.mark.parametrize(('memType'), [
                             ('flash'),
                             ('ram')
                             ])
    def test_sb_write_unaligned_address(self, bl, memType):
        sb_commandDictionay['writeMemory'].cumulativeWrite = False

        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        alignedBase = bl.target.programAlignmentSize
        for offset in range(1, alignedBase):
            sb_commandDictionay['writeMemory'].length = length
            sb_commandDictionay['writeMemory'].dataType = 'file_bin'
            sb_commandDictionay['writeMemory'].startAddress = startAddress + offset
            sb_commandDictionay['writeMemory'].endAddress = startAddress + length + offset
            sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, sb_commandDictionay['writeMemory'].startAddress, length)
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
            status, results = bl.receive_sb_file(sbFilePath)
            if memType == 'flash':
                assert status == bootloader.status.kStatus_FlashAlignmentError
            elif memType == 'ram':
                assert status == bootloader.status.kStatus_Success

    @pytest.mark.parametrize(('memType'), [
                             ('flash'),
                             ('ram')
                             ])
    def test_sb_cumulative_write(self, bl, memType):
        sb_commandDictionay['writeMemory'].cumulativeWrite = True

        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        sb_commandDictionay['writeMemory'].length = length
        sb_commandDictionay['writeMemory'].dataType = 'file_bin'
        sb_commandDictionay['writeMemory'].startAddress = startAddress
        sb_commandDictionay['writeMemory'].endAddress = startAddress + length
        sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, startAddress, length)
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        if memType == 'flash':
            assert status == bootloader.status.kStatus_FlashCommandFailure or status == bootloader.status.kStatusMemoryCumulativeWrite
        elif memType == 'ram':
            assert status == bootloader.status.kStatus_Success

    @pytest.mark.parametrize(('memType'), [
                             ('flash'),
                             ('ram')
                             ])
    def test_sb_write_memory_out_of_range(self, bl, memType):
        sb_commandDictionay['writeMemory'].cumulativeWrite = False

        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        sb_commandDictionay['writeMemory'].length = length
        sb_commandDictionay['writeMemory'].dataType = 'file_bin'
        # Make sure the start_addr is anligned and near the end of the memory region.
        sb_commandDictionay['writeMemory'].startAddress = endAddress + 1 - bl.target.programAlignmentSize
        sb_commandDictionay['writeMemory'].endAddress = sb_commandDictionay['writeMemory'].startAddress + length
        sb_commandDictionay['writeMemory'].data = common_util.generate_random_data_file(bl, startAddress, length)
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatusMemoryRangeInvalid


class TestSbCommandFillMemory:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('memType', 'pattern_format'), [
                             ('flash', 'byte'),
                             ('flash', 'short'),
                             ('flash', 'word'),
                             ('ram', 'byte'),
                             ('ram', 'short'),
                             ('ram', 'word')
                             ])
    def test_sb_fill_all_available_memory(self, bl, memType, pattern_format):
        sb_commandDictionay['fillMemory'].cumulativeFill = False

        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # ---------------------------------------------------------------------------------
        if pattern_format == 'byte':
            pattern = kFilledValue & 0xFF
        elif pattern_format == 'short':
            pattern = kFilledValue & 0xFFFF
        elif pattern_format == 'word':
            pattern = kFilledValue & 0xFFFFFFFF
        # ---------------------------------------------------------------------------------
        sb_commandDictionay['fillMemory'].pattern = pattern
        sb_commandDictionay['fillMemory'].patternFormat = pattern_format
        sb_commandDictionay['fillMemory'].startAddress = startAddress
        sb_commandDictionay['fillMemory'].endAddress = startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'fillMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success
        # Read back the filling data and compare with the filling data.
        read_file = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(startAddress, length, read_file)
        assert common_util.is_fill_memory_correct(read_file, length, pattern, pattern_format) == True

    @pytest.mark.parametrize(('memType'), [
                            ('flash'),
                            ('ram')
                            ])
    def test_sb_fill_all_reserved_region(self, bl, memType):
        sb_commandDictionay['fillMemory'].cumulativeFill = False

        startAddress, endAddress, length = common_util.get_reserved_memory_region(bl, memType)
        if length == 0:
            pass    # ROM has no reserved flash region
        else:
            sb_commandDictionay['fillMemory'].pattern = kFilledValue
            sb_commandDictionay['fillMemory'].patternFormat = 'word'
            sb_commandDictionay['fillMemory'].startAddress = startAddress
            sb_commandDictionay['fillMemory'].endAddress = startAddress + length
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'fillMemory')
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatusMemoryRangeInvalid

    @pytest.mark.parametrize(('memType'), [
                            ('flash'),
                            ('ram')
                            ])
    def test_sb_fill_unaligned_address(self, bl, memType):
        sb_commandDictionay['fillMemory'].cumulativeFill = False
        sb_commandDictionay['fillMemory'].pattern = kFilledValue
        sb_commandDictionay['fillMemory'].patternFormat = 'word'

        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        alignedBase = bl.target.programAlignmentSize
        for offset in range(1, alignedBase):
            sb_commandDictionay['fillMemory'].startAddress = startAddress + offset
            sb_commandDictionay['fillMemory'].endAddress = startAddress + length + offset
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'fillMemory')
            status, results = bl.receive_sb_file(sbFilePath)
            if memType == 'flash':
                assert status == bootloader.status.kStatus_FlashAlignmentError
            elif memType == 'ram':
                assert status == bootloader.status.kStatus_Success

    @pytest.mark.parametrize(('memType'), [
                            ('flash'),
                            ('ram')
                            ])
    def test_sb_cumulative_fill(self, bl, memType):
        sb_commandDictionay['fillMemory'].cumulativeFill = True
        sb_commandDictionay['fillMemory'].pattern = kFilledValue
        sb_commandDictionay['fillMemory'].patternFormat = 'word'

        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        sb_commandDictionay['fillMemory'].startAddress = startAddress
        sb_commandDictionay['fillMemory'].endAddress = startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'fillMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        if memType == 'flash':
            assert status == bootloader.status.kStatus_FlashCommandFailure or status == bootloader.status.kStatusMemoryCumulativeWrite
        elif memType == 'ram':
            assert status == bootloader.status.kStatus_Success

    @pytest.mark.parametrize(('memType'), [
                            ('flash'),
                            ('ram')
                            ])
    def test_sb_fill_memory_out_of_range(self, bl, memType):
        sb_commandDictionay['fillMemory'].cumulativeFill = False
        sb_commandDictionay['fillMemory'].pattern = kFilledValue
        sb_commandDictionay['fillMemory'].patternFormat = 'word'
        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        # Make sure the start_addr is anligned and near the end of the memory region.
        sb_commandDictionay['fillMemory'].startAddress = endAddress + 1 - bl.target.programAlignmentSize
        sb_commandDictionay['fillMemory'].endAddress = sb_commandDictionay['fillMemory'].startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'fillMemory')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatusMemoryRangeInvalid

class TestSbCommandFlashEraseRegion:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('availableFlashSize'), [
                             ('zero'),
                             ('oneSectorSize'),
                            ('allAvailableSize')
                            ])
    def test_sb_erase_available_flash_region(self, bl, availableFlashSize):
        startAddress, endAddress, length = common_util.get_available_memory_region(bl, 'flash')
        # 1. Get actual erased length according to the parameter
        if availableFlashSize == 'zero':
            length = 0
        elif availableFlashSize == 'oneSectorSize':
            length = common_util.get_flash_sector_size(bl)
        elif availableFlashSize == 'allAvailableSize':
            length = length
        # 2. erase with blhost command
        status, results = bl.flash_erase_region(startAddress, length)
        assert status == bootloader.status.kStatus_Success
        # 3. write random data to [startAddress, endAddress]
        randomFile = common_util.generate_random_data_file(bl, startAddress, length)
        status, results = bl.write_memory(startAddress, randomFile)
        assert status == bootloader.status.kStatus_Success
        # 4. generate sb file and erase with sb command
        sb_commandDictionay['flashEraseRegion'].startAddress = startAddress
        sb_commandDictionay['flashEraseRegion'].endAddress = startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseRegion')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success
        # 5. read data from [startAddress, endAddress]
        filePath = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results == bl.read_memory(startAddress, length, filePath)
        assert status == bootloader.status.kStatus_Success
        # 6. verify all the data are FF
        flag = True
        fileObj = open(filePath, 'rb')
        for i in range(0, length):
           fileObj.seek(i)
           data_tmp = fileObj.read(1)
           if ord(data_tmp) != 0xFF:
               flag = False
               break
        fileObj.close()
        assert flag == True

        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            # 7. unsecure the flash
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success
        else:
            pass

    @pytest.mark.parametrize(('availableRamSize'), [
                             ('zero'),
                             ('oneSectorSize'),
                            ('allAvailableSize')
                            ])
    def test_sb_erase_available_ram_region(self, bl, availableRamSize):
        startAddress, endAddress, length = common_util.get_available_memory_region(bl, 'ram')
        # 1. Get actual erased length according to the parameter
        if availableRamSize == 'zero':
            length = 0
        elif availableRamSize == 'oneSectorSize':
            length = common_util.get_flash_sector_size(bl)
        elif availableRamSize == 'allAvailableSize':
            length = length
        # 2. generate sb file and erase with sb command
        sb_commandDictionay['flashEraseRegion'].startAddress = startAddress
        sb_commandDictionay['flashEraseRegion'].endAddress = startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseRegion')
        status, results = bl.receive_sb_file(sbFilePath)
        if length == 0:
            assert status == bootloader.status.kStatus_Success
        else:
            # erase ram region (including available region and reserved reigion) should return kStatus_FlashAddressError
            assert status == bootloader.status.kStatus_FlashAddressError

    @pytest.mark.parametrize('memType', [
                             ('flash'),
                             ('ram')
                            ])
    def test_sb_erase_all_reserved_region(self, bl, memType):
        startAddress, endAddress, length = common_util.get_reserved_memory_region(bl, memType)
        sb_commandDictionay['flashEraseRegion'].startAddress = startAddress
        sb_commandDictionay['flashEraseRegion'].endAddress = startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseRegion')
        status, results = bl.receive_sb_file(sbFilePath)
        if memType == 'flash':
            # ROM has no reserved region, erase zero length flash should return kStatus_Success, otherwise kStatusMemoryRangeInvalid
            if length == 0:
                assert status == bootloader.status.kStatus_Success
            else:
                assert status == bootloader.status.kStatusMemoryRangeInvalid
        elif memType == 'ram':
            # erase ram region (including available region and reserved reigion) should return kStatus_FlashAddressError
            assert status == bootloader.status.kStatus_FlashAddressError

    @pytest.mark.parametrize('memType', [
                             ('flash'),
                             ('ram')
                            ])
    def test_sb_erase_out_of_range(self, bl, memType):
        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the erase length as 1 sector.
        length = common_util.get_flash_sector_size(bl)
        # Make sure the start_addr is anligned and near the end of the memory region.
        sb_commandDictionay['flashEraseRegion'].startAddress = endAddress + 1 - bl.target.eraseAlignmentSize
        sb_commandDictionay['flashEraseRegion'].endAddress = sb_commandDictionay['flashEraseRegion'].startAddress + length
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseRegion')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatusMemoryRangeInvalid

    @pytest.mark.parametrize('memType', [
                             ('flash'),
                             ('ram')
                            ])
    def test_sb_erase_unaligned_address(self, bl, memType):
        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        alignBase = bl.target.eraseAlignmentSize
        for i in range(1, alignBase):
            sb_commandDictionay['flashEraseRegion'].startAddress = startAddress + i
            sb_commandDictionay['flashEraseRegion'].endAddress = sb_commandDictionay['flashEraseRegion'].startAddress + length
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseRegion')
            status, results = bl.receive_sb_file(sbFilePath)
            if memType == 'flash':
                assert status == bootloader.status.kStatus_FlashAlignmentError
            elif memType == 'ram':
                # erase ram region (including available region and reserved reigion) should return kStatus_FlashAddressError
                assert status == bootloader.status.kStatus_FlashAddressError

    @pytest.mark.parametrize('memType', [
                             ('flash'),
                             ('ram')
                            ])
    def test_sb_erase_unaligned_length(self, bl, memType):
        startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
        # Here we just need a small amount data for this case. Set the length as 1KB.
        length = 0x400
        alignBase = bl.target.eraseAlignmentSize
        for i in range(1, alignBase):
            sb_commandDictionay['flashEraseRegion'].startAddress = startAddress
            sb_commandDictionay['flashEraseRegion'].endAddress = startAddress + length + i
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseRegion')
            status, results = bl.receive_sb_file(sbFilePath)
            if memType == 'flash':
                assert status == bootloader.status.kStatus_FlashAlignmentError
            elif memType == 'ram':
                # erase ram region (including available region and reserved reigion) should return kStatus_FlashAddressError
                assert status == bootloader.status.kStatus_FlashAddressError


class TestSbCommandFlashEraseAllUnsecure:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_sb_flash_erase_unsecure(self, bl):
        if bl.target.bootloaderType != bootsources.kBootROM_ExecuteROM:
            pytest.skip('This case only supports the ROM-resident bootloader.')
        else:
            # 1. erase all the flash
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
            # 2. generate sb file with "erase unsecure all" and send the sb file to target
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseAllUnsecure')
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatus_Success
            # 3. reset target
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)
            # 4. check the flash security state, should be unsecure.
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 0

class TestSbCommandFlashEraseAll:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_sb_erase_all_internal_flash(self, bl):
        # 1. generate sb file with "erase all" and send the sb file to target, should be success
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'internalFlashEraseAll')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success
        # 2. get flash region that can be erased by 'erase all' with sb file
        startAddress, endAddress, length = common_util.get_available_memory_region(bl, 'flash')
        # 3. write random data to [startAddress, endAddress], should be success
        randomFile = common_util.generate_random_data_file(bl, startAddress, length)
        status, results = bl.write_memory(startAddress, randomFile)
        assert status == bootloader.status.kStatus_Success
        # 4. erase all the internal flash with sb file, should be success
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success
        # 5. read data from [startAddress, endAddress]
        filePath = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results == bl.read_memory(startAddress, length, filePath)
        assert status == bootloader.status.kStatus_Success
        # 6. verify all the data are FF
        flag = True
        fileObj = open(filePath, 'rb')
        for i in range(0, length):
           fileObj.seek(i)
           data_tmp = fileObj.read(1)
           if ord(data_tmp) != 0xFF:
               flag = False
               break
        fileObj.close()
        assert flag == True

        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            # 7. reset the target, flash should be in secure state
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 1
            # 8. unsecure the flash
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success
        else:
            pass


class TestSbCommandJump:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_sb_jump_to_entry_point(self, bl):
        # .out and .elf file has entry point while .bin file doesn't have.
        (elfFile, hexFile, binFile) = common_util.get_led_demo_path(bl)
        sb_commandDictionay['writeMemory'].cumulativeWrite = False
        sb_commandDictionay['writeMemory'].data = elfFile
        sb_commandDictionay['writeMemory'].dataType = 'app_out'
        # Generate sb file
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory', 'jumpEntryPoint')
        # After excuting receive-sb-file the led will blink immediately and blhost returns kStatus_AbortDataPhase.
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_AbortDataPhase
        # Send command to blhost, it will fail
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        status != bootloader.status.kStatus_Success
        # Let led blink for some time so that we can see the correct phenomenon.
        time.sleep(3)

    def test_sb_jump_to_pc_address(self, bl):
        # Extract PC address from app demo bin file, and then jump to PC.
        # The case can pass on K80 while fail on L5K. Jump will not support jump to explicit address, but only support jump to app entry point
        (elfFile, hexFile, binFile) = common_util.get_led_demo_path(bl)
            
        sb_commandDictionay['writeMemory'].cumulativeWrite = False
        sb_commandDictionay['writeMemory'].data = binFile
        sb_commandDictionay['writeMemory'].dataType = 'app_bin'
        sb_commandDictionay['writeMemory'].startAddress = bl.target.BL_APP_VECTOR_TABLE_ADDRESS
        sb_commandDictionay['writeMemory'].endAddress = sb_commandDictionay['writeMemory'].startAddress + os.path.getsize(binFile)

        # Generate sb file
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory', 'jumpEntryPoint')
        # After excuting receive-sb-file the led will blink immediately and blhost returns kStatus_AbortDataPhase.
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_AbortDataPhase
        # Send command to blhost, it will fail
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        status != bootloader.status.kStatus_Success
        # Let led blink for some time so that we can see the correct phenomenon.
        time.sleep(3)


    @pytest.mark.parametrize(('hasArg'),[
                             (False),
                             (True)
                            ])
    def test_sb_jump_to_sp_pc_with_arg(self, bl, hasArg):
        # Extract SP and PC address from app demo bin file, and then jump to SP and PC without argument
        (elfFile, hexFile, binFile) = common_util.get_led_demo_path(bl)
    
        sb_commandDictionay['writeMemory'].cumulativeWrite = False
        sb_commandDictionay['writeMemory'].data = binFile
        sb_commandDictionay['writeMemory'].dataType = 'app_bin'
        sb_commandDictionay['writeMemory'].startAddress = bl.target.BL_APP_VECTOR_TABLE_ADDRESS
        sb_commandDictionay['writeMemory'].endAddress = sb_commandDictionay['writeMemory'].startAddress + os.path.getsize(binFile)

        sb_commandDictionay['jumpStackPoint'].hasArg = hasArg
        if hasArg:
            # Set the argument as random 32-bit data if has.
            sb_commandDictionay['jumpStackPoint'].arg = random.randint(0, 0xffffffff)
        else:
            sb_commandDictionay['jumpStackPoint'].arg = None
        # Generate sb file
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory', 'jumpStackPoint')
        # After excuting receive-sb-file the led will blink immediately and blhost returns kStatus_AbortDataPhase.
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_AbortDataPhase
        # Send command to blhost, it will fail
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        status != bootloader.status.kStatus_Success
        # Let led blink for some time so that we can see the correct phenomenon.
        time.sleep(3)



class TestSbCommandProgramIFR:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    # @brief Programming the erasable IFR (index = 0x30 ~ 0x33) if has.
    def test_sb_program_4bytes_ifr(self, bl):
        if not bl.target.hasErasableIFR:
            pytest.skip('\nThis chip has no erasable IFR.\n')
        else:
            # Progam the erasable IFR from index 0x30 ~ 0x33
            sb_commandDictionay['programIfr'].ifrIndex1 = 0x30
            sb_commandDictionay['programIfr'].ifrValue1 = 0xFFFFFFFF
            sb_commandDictionay['programIfr'].ifrIndex2 = 0x31
            sb_commandDictionay['programIfr'].ifrValue2 = 0xFFFFFFFF
            sb_commandDictionay['programIfr'].ifrIndex3 = 0x32
            sb_commandDictionay['programIfr'].ifrValue3 = 0xFFFFFFFF
            sb_commandDictionay['programIfr'].ifrIndex4 = 0x33
            sb_commandDictionay['programIfr'].ifrValue4 = 0xFFFFFFFF
            # Generate sb file
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'programIfr')
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatus_Success
                
            # Read back the IFR values from the index 0x30 to 0x33, and verify the program values.
            status, results = bl.flash_read_once(sb_commandDictionay['programIfr'].ifrIndex1, 4)
            assert status == bootloader.status.kStatus_Success
            assert results[1] == sb_commandDictionay['programIfr'].ifrValue1
            status, results = bl.flash_read_once(sb_commandDictionay['programIfr'].ifrIndex2, 4)
            assert status == bootloader.status.kStatus_Success
            assert results[1] == sb_commandDictionay['programIfr'].ifrValue2
            status, results = bl.flash_read_once(sb_commandDictionay['programIfr'].ifrIndex3, 4)
            assert status == bootloader.status.kStatus_Success
            assert results[1] == sb_commandDictionay['programIfr'].ifrValue3
            status, results = bl.flash_read_once(sb_commandDictionay['programIfr'].ifrIndex4, 4)
            assert status == bootloader.status.kStatus_Success
            assert results[1] == sb_commandDictionay['programIfr'].ifrValue4

#     def test_sb_program_8bytes_ifr(self, bl):
#         # 8 bytes IFR is not erasable, it can only be programmed once. Here we pass this test case.
#         pass

class TestSbCommandCall:
    global SYST_RVR_ADDR
    SYST_RVR_ADDR = 0xE000E014
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('hasArg'),[
                             (False),
                             (True)
                            ])
    def test_sb_call_a_function_with_arg(self, bl, hasArg):
        ''' 
        Step 1: Design a function, below is the code in the main.c file.
        ==================================== main.c (start)==========================================
        #include <stdio.h>
        #include <stdint.h>
        #define SysTick_BASE_ADDR       0xE000E010
        #define SYST_RVR_OFFSET         0x04
        #define SYST_RVR_ADDR           (SysTick_BASE_ADDR + SYST_RVR_OFFSET)
        //This function to program the SysTick Reload Value Register (SYST_RVR), which is a 24-bit register.
        //Scenario 1: with one input arguement.
        int program_SYST_RVR_register_with_arg(unsigned int reload_value)
        {
            *((volatile uint32_t*)SYST_RVR_ADDR) = reload_value;
            return 0;
        }
        //Scenario 2: with no input arguement.
        int program_SYST_RVR_register_without_arg(void)
        {
            *((volatile uint32_t*)SYST_RVR_ADDR) = 0x123456; // Program the fix value
            return 0;
        }
        int main()
        {
            return 0;
        }
        ==================================== main.c (end)==========================================
        Step 2: Create an IAR project and add the main.c to the project.
        Step 3: Set the IAR Options-->Linker-->Library-->Entry symbol as the function name 
                (i.e program_SYST_RVR_register_with_arg or program_SYST_RVR_register_without_arg)
        Step 4: Modify the default linker file (Options-->Linker-->Config-->Edit-->Memory Regions-->IROM1 Start: 0xA000)
        Step 5: Build the project and generate the out file 
                (program_SYST_RVR_register_with_arg.out or program_SYST_RVR_register_without_arg).
        '''
        print("Use Jlink to read the SysTick Reload Value Register(24bits) and get the original value:")
        originalValueBinFile = bl.target.dump_memory(SYST_RVR_ADDR, 4)
        fileObj = file(originalValueBinFile, 'rb')
        data = fileObj.read()
        fileObj.close()
        originalValue = ord(data[0]) | (ord(data[1]) << 8) | (ord(data[2]) << 16) | (ord(data[3]) << 24)
        print("The original value of SysTick Reload Value Register: 0x%x" %(originalValue))
        
        if hasArg:
            sb_commandDictionay['call'].hasArg = True
            sb_commandDictionay['call'].arg = random.randint(0, 0x00ffffff)
            outFilePath = os.path.join(bl.vectorsDir,'call_function', 'program_SYST_RVR_register_with_arg.out')
        else:
            sb_commandDictionay['call'].hasArg = False
            sb_commandDictionay['call'].arg = 0x123456
            outFilePath = os.path.join(bl.vectorsDir,'call_function', 'program_SYST_RVR_register_without_arg.out')
        if not os.path.exists(outFilePath):
            print("Cannot find the out file: %s" %outFilePath)
            raise ValueError("Cannot find the out file: %s" %outFilePath)
        else:
            # Initialize the dictionary
            sb_commandDictionay['writeMemory'].cumulativeWrite = False
            sb_commandDictionay['writeMemory'].data = outFilePath
            sb_commandDictionay['writeMemory'].dataType = 'app_out' 
            
            sb_commandDictionay['flashEraseRegion'].startAddress = 0xA000
            sb_commandDictionay['flashEraseRegion'].endAddress   = 0xB000
            
            # generate sb file
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'flashEraseRegion', 'writeMemory', 'call')
            print("Call a function to program 0x%x to the SysTick Reload Value Register in the sb file:" %(sb_commandDictionay['call'].arg)),
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatus_Success
            os.remove(sbFilePath)
            
            '''Note: Here we can use both JLink and blhost command to read the SysTick Reload Value Register,
                     but better use JLink, because for some platforms such as L5K, this register is not in the 
                     defined regions in g_memoryMap, which is defined in memory_map_#platform.c file, so when 
                     using read-mmeory to read this register, blhost always returns kStatusMemoryRangeInvalid.
            '''
            # Read SysTick Reload Value Register(24bits) and compare it with the input arg, should be equal.
            programmedValueBinFile = bl.target.dump_memory(SYST_RVR_ADDR, 4)
            fileObj = file(programmedValueBinFile, 'rb')
            data = fileObj.read()
            fileObj.close()
            programmedValue = ord(data[0]) | (ord(data[1]) << 8) | (ord(data[2]) << 16) | (ord(data[3]) << 24)
            print("After calling the function, the value of SysTick Reload Value Register is changing to 0x%x" %(programmedValue))
            if programmedValue == sb_commandDictionay['call'].arg:
                print("0x%x == 0x%x, call the function successfully!" %(programmedValue, sb_commandDictionay['call'].arg))
                testResult = True
            else:
                print("0x%x != 0x%x, call the function fail!" %(programmedValue, sb_commandDictionay['call'].arg))
                testResult = False
            assert testResult == True
            
            print("Use Jlink to write the original value back to the SysTick Reload Value Register(24bits):")
            bl.target.restore_memory(SYST_RVR_ADDR, originalValueBinFile)
            

class TestSbCommandReset:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_sb_reset_target(self, bl):
        # if app not exist, just generate reset command in sb file
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'reset')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_AbortDataPhase
#         if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
#             # load led demo bin file to flash and reset target with sb file
#             app_bin_exists, led_demo_bin_path = common_util.get_led_demo_path(bl, 'app_bin')
#             if app_bin_exists:
#                 sb_commandDictionay['writeMemory'].cumulativeWrite = False
#                 sb_commandDictionay['writeMemory'].data = led_demo_bin_path
#                 sb_commandDictionay['writeMemory'].dataType = 'app_bin'
#                 sb_commandDictionay['writeMemory'].startAddress = bl.target.BL_APP_VECTOR_TABLE_ADDRESS
#                 sb_commandDictionay['writeMemory'].endAddress = sb_commandDictionay['writeMemory'].startAddress + os.path.getsize(led_demo_bin_path)
#                 # If app exists, load app to flash
#                 sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'writeMemory', 'reset')
#                 status, results = bl.receive_sb_file(sbFilePath)
#                 assert status == bootloader.status.kStatus_AbortDataPhase
#                 # Wait about 5s, the led will blink.
#                 time.sleep(8)
#                 # Send command to blhost, it will fail
#                 status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
#                 status != bootloader.status.kStatus_Success
#             else:
#                 # if app not exist, just generate reset command in sb file
#                 sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', 'reset')
#                 status, results = bl.receive_sb_file(sbFilePath)
#                 assert status == bootloader.status.kStatus_AbortDataPhase
#         else:
#             pytest.skip('\nDo not support reset sb command.\n')



# class TestsbCommand_qspiFlashEraseAll:
#     '''Move this case to test_quadspi.py'''
#     pass
#
# class TestsbCommand_enableQspi:
#     '''Move this case to test_quadspi.py'''
#     pass
#
# class Testsb_command_otfadEncryption:
#     # TODO better put these test cases in an independent py file
#     pass
