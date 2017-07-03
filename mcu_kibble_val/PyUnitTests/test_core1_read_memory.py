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
from common import common_util
from common import read_memory

#############################################################################################################
#                                NOTE
# Read available memory region has been included in the fill-memory and write-memory test cases,
# each time when verifying the fill/write result, read-memry is always used, so, there is 
# no need to repeatedly test reading available memory region here.
#
############################################################################################################# 
@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if(tgt.isDualCoreBootSupported == False):
        pytest.skip('%s does not support dual core boot!' %tgt.targetName)

#############################################################################################################
#
#                Read reserved flash/ram region with core 1.
#
#############################################################################################################        
class TestCore1ReadReservedMemory:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.setup_test_environment(bl, bootCore = 'core1', needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
      
    def test_core1_read_reserved_ram(self, bl):
        read_memory.read_reserved_memory_region(bl, memType = 'ram')
    
    def test_core1_read_reserved_flash(self, bl):
        read_memory.read_reserved_memory_region(bl, memType = 'flash')
    

#############################################################################################################
#
#                Read Flash 0 with core 1.
#
############################################################################################################# 
class TestCore1ReadFlash0:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'flash'
        self.memIndex = 0
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    def test_core1_read_unaligned_flash0_address(self, bl):
        read_memory.read_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_read_flash0_unaligned_byte_length(self, bl):
        read_memory.read_unaligned_bytes_length(bl, self.memType, self.memIndex)
    
    def test_core1_read_out_of_flash0(self, bl):
        read_memory.read_out_of_memory_range(bl, self.memType, self.memIndex)

#############################################################################################################
#
#                Read SRAM 0 with core 1.
#
############################################################################################################# 
class TestCore1ReadSram0:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'ram'
        self.memIndex = 0
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    def test_core1_read_unaligned_ram0_address(self, bl):
        read_memory.read_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_read_ram0_unaligned_byte_length(self, bl):
        read_memory.read_unaligned_bytes_length(bl, self.memType, self.memIndex)
    
    def test_core1_read_out_of_ram0(self, bl):
        read_memory.read_out_of_memory_range(bl, self.memType, self.memIndex)

#############################################################################################################
#
#                Read Flash 1 with core 1.
#
############################################################################################################# 
class TestCore1ReadFlash1:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'flash'
        self.memIndex = 1
        if(self.memIndex >= bl.target.maxFlashIndex):
            pytest.skip("No FLASH 1 region!")
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    def test_core1_read_unaligned_flash1_address(self, bl):
        read_memory.read_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_read_flash1_unaligned_byte_length(self, bl):
        read_memory.read_unaligned_bytes_length(bl, self.memType, self.memIndex)
    
    def test_core1_read_out_of_flash1(self, bl):
        read_memory.read_out_of_memory_range(bl, self.memType, self.memIndex)

#############################################################################################################
#
#                Read SRAM 1 with core 1.
#
############################################################################################################# 
class TestCore1ReadSram1:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'ram'
        self.memIndex = 1
        if(self.memIndex >= bl.target.maxSramIndex):
            pytest.skip("No SRAM 1 region!")
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    def test_core1_read_unaligned_ram1_address(self, bl):
        read_memory.read_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_read_ram1_unaligned_byte_length(self, bl):
        read_memory.read_unaligned_bytes_length(bl, self.memType, self.memIndex)
    
    def test_core1_read_out_of_ram1(self, bl):
        read_memory.read_out_of_memory_range(bl, self.memType, self.memIndex)

#############################################################################################################
#
#                Read SRAM 2 with core 1.
#
############################################################################################################# 
class TestCore1ReadSram2:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'ram'
        self.memIndex = 2
        if(self.memIndex >= bl.target.maxSramIndex):
            pytest.skip("No SRAM 2 region!")
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
 
    def test_core1_read_unaligned_ram2_address(self, bl):
        read_memory.read_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_read_ram2_unaligned_byte_length(self, bl):
        read_memory.read_unaligned_bytes_length(bl, self.memType, self.memIndex)
    
    def test_core1_read_out_of_ram2(self, bl):
        read_memory.read_out_of_memory_range(bl, self.memType, self.memIndex)

##################################################################################
# EOF
##################################################################################