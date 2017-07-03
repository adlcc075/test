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
import time

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from common import common_util

class Testflash_erase_all:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_flash_erase_all_when_security_disabled(self, bl):
        if bl.target.bootloaderType == bootsources.kBootFlash_ExecuteFlash:
            # 1. Erase all the flash, should return success
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
            # 2. Reset the target
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)
            # 3. Check the flash security state, should be unsucure
            # Note: For the flash-resident bootloader, flash has reserved region, the flash configuration
            # area cannot be erased by flash-erase-all command
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 0
            # 4. Flash can be accessed when chip is in unsecure state
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
        else:
            pytest.skip("This case is not supported for the other type of bootloader.")

    def test_flash_erase_all_when_security_enabled(self, bl):
        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            # 1. Erase all the flash
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
            # 2. reset target, the chip will be secure
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            # 3. delay some time after reset
            time.sleep(2)
            # 4. Erase all the flash again, the command is disallowed because security is enabled
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_SecurityViolation
            # 5. Check the flash security state, should be sucure
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 1
            # 6. Unsecure the chip
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success
        else:
            pytest.skip("This case is not supported for the other type of bootloader.")
