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
import random
import time

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from common import common_util
from PyUnitTests import bltest_config

## @brief Test the bootloader get-property commands.
class Testget_property:
    @pytest.fixture(scope = 'function', autouse = True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)

        def teardown():
            pass
        request.addfinalizer(teardown)

    ## @brief Bootloader version (get-property 1).
    def test_blhost_version(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['bootloaderVersion']

    ## @brief Available peripherals (get-property 2)
    def test_available_peripherals(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_AvailablePeripherals)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['availablePeripherals']

    ## @brief Start of program flash (get-property 3)
    def test_flash_start_address(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashStartAddress)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['flashStartAddress']

    ## @brief Size of program flash (get-property 4)
    def test_size_of_flash(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSizeInBytes)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['flashTotalSize']

    ## @brief Size of flash sector (get-property 5)
    def test_size_of_sector(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSectorSize)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['flashSectorSize']

    ## @brief Blocks in flash array (get-property 6)
    def test_blocks_in_flash(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashBlockCount)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['flashBlockCount']

    ## @brief Available commands (get-property 7)
    def test_available_commands(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_AvailableCommands)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['availableCommands']

    ## @brief CRC check status (get-property 8)
    def test_crc_check_status(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CrcCheckStatus)
#         if bl.target['isCRCCheckSupported'] == True:
        if bl.target.isCRCCheckSupported == True:
            assert status == bootloader.status.kStatus_Success
        else:
            assert status == bootloader.status.kStatus_UnknownProperty

    ## @brief Verify Writes flag (get-property 10)
    def test_verify_write_flag(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_VerifyWrites)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['verifyWriteFlag']

    ## @brief Max supported packet size (get-property 11)
    def test_supported_packet_size(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_MaxPacketSize)
        assert status == bootloader.status.kStatus_Success
        assert results[0] == 32

    ## @brief Reserved regions (get-property 12)
    def test_reserved_regions(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_ReservedRegions)
        assert status == bootloader.status.kStatus_Success

    ## @brief Validate regions flag (get-property 13)
    def test_validate_region_flag(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_ValidateRegions)
        assert status == bootloader.status.kStatus_Success or \
               status == bootloader.status.kStatus_UnknownProperty

    ## @brief Start of RAM (get-property 14)
    def test_start_address_of_ram(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_RAMStartAddress)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['ramStartAddress']

    ## @brief Size of RAM (get-property 15)
    def test_size_of_ram(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_RAMSizeInBytes)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['ramTotalSize']

    ## @brief System device identification (get-property 16)
    def test_system_device_id(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_SystemDeviceIdent)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['SystemDeviceId']

    ## @brief Flash security state (get-property 17)
    def test_flash_security_state(self, bl):
#         if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            # Set flash in unsecure state
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success

            # Get the flash security status after flash-erase-all-unsecure
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 0

            # Erase the whole flash
            status, results == bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success

            # Should be in unsecure state before reset target
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 0

            # Reset target
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success

            # Should delay some time after reset when using USB peripheral.
            if bltest_config.peripheral == 'usb':
                time.sleep(2)

            # Should be in secure state after reset target
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 1

            # Set flash in unsecure state
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success
        else:
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 0

    ## @brief Unique device identification (get-property 18)
    def test_unique_device_id(self, bl):
#         if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            pytest.skip('UID is programmed after the ROM chip tape-out.')

        status, results = bl.get_property(bootloader.properties.kPropertyTag_UniqueDeviceIdent)
        assert status == bootloader.status.kStatus_Success

    ## @brief FAC support flag (get-property 19)
    def test_FAC_support_flag(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FacSupportFlag)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['FACSupportFlag']

    ## @brief FAC segment size (get-property 20)
    def test_FAC_segment_size(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FacSegmentSize)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['FACSegmentSize']

    ## @brief FAC segment count (get-property 21)
    def test_FAC_segment_count(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FacSegmentCount)
        assert status == bootloader.status.kStatus_Success
#         assert results[0] == bl.target['FACSegmentCount']

    ## @brief Read margin level of program flash (get-property 22)
    def test_read_margin_level_of_pflash(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashReadMargin)
        assert status == bootloader.status.kStatus_Success or status == bootloader.status.kStatus_UnknownProperty
#         if status == bootloader.status.kStatus_Success:
#             assert results[0] == bl.target['flashReadMarginLevel']

    ## @brief QuadSpi initialization status / Invalid property (get-property 23)
    def test_QSPI_init_status(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_QspiInitStatus)
#         if bl.target['isQspiSupported'] == True:
#             assert status == bootloader.status.kStatus_Success
#         else:
#            assert status == bootloader.status.kStatus_UnknownProperty
        
        assert status == bootloader.status.kStatus_Success or status == bootloader.status.kStatus_UnknownProperty
    ## @brief Target version / Invalid property (get-property 24)
    def test_target_version(self, bl):
        status, results = bl.get_property(bootloader.properties.kPropertyTag_TargetVersion)
#         if bl.target['isTargetVersionSupported'] == True:
#             assert status == bootloader.status.kStatus_Success
#             assert results[0] == bl.target['targetVersion']
#         else:
#            assert status == bootloader.status.kStatus_UnknownProperty
        
        assert status == bootloader.status.kStatus_Success or status == bootloader.status.kStatus_UnknownProperty
                  

    ## @brief Invalid property (tag = 9)
    def test_invalid_property_tag9(self, bl):
        status, results = bl.get_property(9)
        assert status == bootloader.status.kStatus_UnknownProperty

    ## @brief Invalid property (tag = 25~255)
    def test_invalid_property(self, bl):
        status, results = bl.get_property(random.randint(25, 255))
        assert status == bootloader.status.kStatus_UnknownProperty

