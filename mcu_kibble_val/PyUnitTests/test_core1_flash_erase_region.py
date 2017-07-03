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
from common import flash_erase_region

@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if(tgt.isDualCoreBootSupported == False):
        pytest.skip('%s does not support dual core boot!' %tgt.targetName)
        

#############################################################################################################
#
#                Erase reserved flash/ram region with core 1.
#
##############################################################################################################        
class TestCore1EraseReservedMemory:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.setup_test_environment(bl, bootCore = 'core1', needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
    
    def test_core1_erase_reserved_ram(self, bl):
        flash_erase_region.erase_reserved_memory_region(bl, memType = 'ram')
    
    def test_core1_erase_reserved_flash(self, bl):
        flash_erase_region.erase_reserved_memory_region(bl, memType = 'flash')

#############################################################################################################
#
#                Erase Flash 0 with core 1.
#
############################################################################################################# 
class TestCore1EraseFlash0:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'flash'
        self.memIndex = 0
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=True)
            
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_core1_erase_start_available_flash0_1_sector(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=1)
     
    def test_core1_erase_start_available_flash0_2_sectors(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=2)                                                    
     
    def test_core1_erase_start_available_flash0_3_sectors(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=3)
         
    def test_core1_erase_start_available_flash0_4_sectors(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=4)                                                   
       
    def test_core1_erase_half_of_available_flash0(self, bl):   
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, 'halfOfAvailableMemory')
     
    def test_core1_erase_all_of_available_flash0(self, bl):   
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, 'allOfAvailableMemory')

    def test_core1_erase_flash0_unaligned_address(self, bl):
        flash_erase_region.erase_unaligned_memory_address(bl, self.memType, self.memIndex)

    def test_core1_erase_flash0_unaligned_byte_length(self, bl):     
        flash_erase_region.erase_unaligned_bytes_length(bl, self.memType, self.memIndex)

    def test_core1_erase_out_of_flash0(self, bl):
        flash_erase_region.erase_out_of_memory_range(bl, self.memType, self.memIndex)

#############################################################################################################
#
#                Erase Flash 1 with core 1.
#
############################################################################################################# 
class TestCore1EraseFlash1:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'flash'
        self.memIndex = 1
        if(self.memIndex >= bl.target.maxFlashIndex):
            pytest.skip("No FLASH 1 region!")
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=True)
            
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_core1_erase_start_available_flash1_1_sector(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=1)
     
    def test_core1_erase_start_available_flash1_2_sectors(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=2)                                                    
     
    def test_core1_erase_start_available_flash1_3_sectors(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=3)
         
    def test_core1_erase_start_available_flash1_4_sectors(self, bl):
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, sectorsNumber=4)                                                   
       
    def test_core1_erase_half_of_available_flash1(self, bl):   
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, 'halfOfAvailableMemory')
     
    def test_core1_erase_all_of_available_flash1(self, bl):   
        flash_erase_region.erase_sectors_at_start_of_available_region(bl, self.memType, self.memIndex, 'allOfAvailableMemory')

    def test_core1_erase_flash1_unaligned_address(self, bl):
        flash_erase_region.erase_unaligned_memory_address(bl, self.memType, self.memIndex)

    def test_core1_erase_flash1_unaligned_byte_length(self, bl):     
        flash_erase_region.erase_unaligned_bytes_length(bl, self.memType, self.memIndex)

    def test_core1_erase_out_of_flash1(self, bl):
        flash_erase_region.erase_out_of_memory_range(bl, self.memType, self.memIndex)