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
from fsl.bootloader import bootsources
from common import common_util
from common import flash_security_disable

@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if(tgt.isDualCoreBootSupported == False):
        pytest.skip('%s does not support dual core boot!' %tgt.targetName)
        
    if(tgt.bootloaderType != bootsources.kBootROM_ExecuteROM):
        pytest.skip('flash-security-disable only supports on ROM-resident bootloader.')

#############################################################################################################
#
#                flash-security-disable with core 1.
#
##############################################################################################################        
class TestCore1FlashSecurityDisable:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        common_util.setup_test_environment(bl, self.bootCore, needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
    
    # Test flash-security-disable with backdoor key all 0s, which is an invalid key.
    def test_core1_flash_security_disable_key_0s(self, bl):
        flash_security_disable.flash_security_disable(bl, backdoorKey = '0000000000000000', bootCore = self.bootCore)

    # Test flash-security-disable with backdoor key all 1s, which is an invalid key.
    def test_core1_flash_security_disable_key_1s(self, bl):
        flash_security_disable.flash_security_disable(bl, backdoorKey = 'FFFFFFFFFFFFFFFF', bootCore = self.bootCore)
             
    # Test flash-security-disable with a formal backdoor key (not all 0s and 1s), which is a valid key.
    def test_core1_flash_security_disable_key_valid(self, bl):
        flash_security_disable.flash_security_disable(bl, backdoorKey = '0123456789ABCDEF', bootCore = self.bootCore)
