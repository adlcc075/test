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


class Testflash_erase_all_unsecure:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_flash_erase_all_unsecure(self, bl):
        # This command only support for ROM-resident bootloader
        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            # 1. Let flash be in secure state
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            if results[0] == 0:
                status, results = bl.flash_erase_all()
                assert status == bootloader.status.kStatus_Success
                status, results = bl.reset()
                assert status == bootloader.status.kStatus_Success
                time.sleep(3)
            # 2. Check if flash is in secure state
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 1
            # 3. flash-erase-all is not supported when flash is in secure state
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_SecurityViolation
            # 4. Unsecure the flash
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success
            # 5. flash-erase-all is supported when flash is in unsecure state
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
            # 6. Set flash in unsecure state finally
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success
        else:
            pytest.skip("This case is not supported for the other type of bootloader.")