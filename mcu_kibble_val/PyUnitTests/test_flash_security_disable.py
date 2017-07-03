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
from common import common_util
from common import flash_config_area


## @brief This function is to covert the 64 bits hex data to string data with length of 16. For example:
# Input 0x0,                get '0000000000000000'
# Input 0x100056,           get '0000000000100056'
# Input 0x0123456789ABCDEF, get '0123456789ABCDEF'
def hex_to_str(hex_data):
    retValue = ''
    print type(retValue)
    for i in range(0, 16):
        if (hex_data & (0xF << (60 - 4*i))) == 0:
            retValue = retValue + '0'
        else:
            retValue = retValue + '%x'%(hex_data)
            break
    return retValue


class Testflash_security_disable:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            pass
        request.addfinalizer(teardown)

    @pytest.mark.parametrize(('backdoorKey'), [
                            # Case 1: Backdoor key is not all 0s and not all 1s
                            (0x0123456789ABCDEF),
                            # Case 2: Backdoor key is all 1s
                            (0xFFFFFFFFFFFFFFFF),
                            # Case 3: Backdoor key is all 0s
                            (0x0000000000000000)
                            ])
    def test_flash_security_disable(self, bl, backdoorKey):
        # This command only support for ROM-resident bootloader
        if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            # 1. Erase all the flash, including the flash configuration data (0x400 ~ 0x40F)
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
            # 2. Generate flash configuration data
            flashConfigDataFile = flash_config_area.generate_flash_config_data(
                    backdoorKeyByte0 = (backdoorKey & 0xFF00000000000000) >> 56,
                    backdoorKeyByte1 = (backdoorKey & 0x00FF000000000000) >> 48,
                    backdoorKeyByte2 = (backdoorKey & 0x0000FF0000000000) >> 40,
                    backdoorKeyByte3 = (backdoorKey & 0x000000FF00000000) >> 32,
                    backdoorKeyByte4 = (backdoorKey & 0x00000000FF000000) >> 24,
                    backdoorKeyByte5 = (backdoorKey & 0x0000000000FF0000) >> 16,
                    backdoorKeyByte6 = (backdoorKey & 0x000000000000FF00) >> 8,
                    backdoorKeyByte7 = (backdoorKey & 0x00000000000000FF),
                    # here should enable backdoor key access
                    flashSecurityRegister       = 0xbf)
            # 3. Update the flash configuration area (0x400 ~ 0x40F)
            status, results = bl.write_memory(0x400, flashConfigDataFile)
            assert status == bootloader.status.kStatus_Success
            # 4. Reset target
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)
            # 5. Check if flash is in secure state
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 1
            # 6. Flash cannot be accessed when it is in secure state
            status, results = bl.flash_erase_region(0, 0x800)
            assert status == bootloader.status.kStatus_SecurityViolation
            # 7. Write the wrong backdoor key, should return kStatus_FlashAccessError
            wrongKey = hex_to_str(random.randint(0,0xFFFFFFFFFFFFFFFF))
            status, results = bl.flash_security_disable(wrongKey)
            assert status == bootloader.status.kStatus_FlashAccessError

            # 8. Write the right backdoor key without reset, should also return kStatus_FlashAccessError
            # -------------------------------------------------------------------------------------------------------------------------
            # Note: if the backdoor keys do not match, security is not released and all future attempts to execute the Verify Backdoor
            #       Access Key command are immediately aborted and the FSTAT[ACCERR] bit, i.e. Flash Access Error Flag is (again) set
            #       to 1 until a reset of the flash memory module occurs.
            # -------------------------------------------------------------------------------------------------------------------------
            rightKey = hex_to_str(backdoorKey)
            status, results = bl.flash_security_disable(rightKey)
            assert status == bootloader.status.kStatus_FlashAccessError
            # 9. Write the right backdoor key after reset, should return kStatus_Success
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)
            status, results = bl.flash_security_disable(rightKey)
            if backdoorKey == 0xFFFFFFFFFFFFFFFF or backdoorKey == 0x0000000000000000:
                # If the entire 8-byte key is all zeros or all ones, the Verify Backdoor
                # Access Key command fails with an access error
                assert status == bootloader.status.kStatus_FlashAccessError
            else:
                assert status == bootloader.status.kStatus_Success
                # 10. Check if flash is in unsecure state
                status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
                assert status == bootloader.status.kStatus_Success
                assert results[0] == 0
                # 11. Flash can be accessed when it is in unsecure state
                status, results = bl.flash_erase_region(0, 0x800)
                assert status == bootloader.status.kStatus_Success
                # 12. Flash will be in secure state after reset
                status, results = bl.reset()
                assert status == bootloader.status.kStatus_Success
                time.sleep(2)
                status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
                assert status == bootloader.status.kStatus_Success
                assert results[0] == 1

            # 13. Set flash in unsecure state finally
            status, results = bl.flash_erase_all_unsecure()
            assert status == bootloader.status.kStatus_Success
        else:
            pytest.skip("This case is not supported on this type of bootloader.")