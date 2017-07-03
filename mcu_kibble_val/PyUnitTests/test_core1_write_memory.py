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
from common import write_memory

@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if(tgt.isDualCoreBootSupported == False):
        pytest.skip('%s does not support dual core boot!' %tgt.targetName)

#############################################################################################################
#
#                Write reserved flash/ram region with core 0.
#
##############################################################################################################        
class TestCore1WriteReservedMemory:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        common_util.setup_test_environment(bl, bootCore = 'core1', needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
      
    def test_core1_write_reserved_ram(self, bl):
        write_memory.write_reserved_memory_region(bl, memType = 'ram')
    
    def test_core1_write_reserved_flash(self, bl):
        write_memory.write_reserved_memory_region(bl, memType = 'flash')

#############################################################################################################
#
#                Write Flash 0 with core 0.
#
############################################################################################################# 
class TestCore1WriteFlash0:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        self.memType = 'flash'
        self.memIndex = 0
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=True)
            
        def teardown():
            pass
        request.addfinalizer(teardown)

    def test_core1_write_available_flash0_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 1)
     
    def test_core1_write_available_flash0_2_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 2)                                                    
     
    def test_core1_write_available_flash0_3_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 3)
         
    def test_core1_write_available_flash0_4_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 4)
     
    def test_core1_write_available_flash0_5_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 5)                                                    
     
    def test_core1_write_available_flash0_6_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 6)
         
    def test_core1_write_available_flash0_1KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x400)
      
    def test_core1_write_available_flash0_1KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x401)                                                    
      
    def test_core1_write_available_flash0_2KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x800)
      
    def test_core1_write_available_flash0_2KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x801)                                                    
      
    def test_core1_write_available_flash0_4KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1000)
      
    def test_core1_write_available_flash0_4KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1001)                                                    
        
    def test_core1_write_half_of_available_flash0(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'halfOfAvailableMemory')
      
    def test_core1_write_all_of_available_flash0(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'allOfAvailableMemory')
  
    def test_core1_write_unaligned_flash0_address(self, bl):
        write_memory.write_unaligned_memory_address(bl, self.memType, self.memIndex)
     
    def test_core1_write_out_of_flash0(self, bl):
        write_memory.write_out_of_memory_range(bl, self.memType, self.memIndex)
     
    def test_core1_cumulative_write_flash0(self, bl):
        write_memory.cumulative_write(bl, self.memType, self.memIndex)
 
#############################################################################################################
#
#                Write SRAM 0 with core 0.
#
#############################################################################################################  
class TestCore1WriteSram0:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl): 
        self.bootCore = 'core1'
        self.memType = 'ram'
        self.memIndex = 0
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll=False)
              
        def teardown():
            pass
        request.addfinalizer(teardown)
       
    def test_core1_write_available_ram0_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 1)
     
    def test_core1_write_available_ram0_2_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 2)                                                    
     
    def test_core1_write_available_ram0_3_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 3)
         
    def test_core1_write_available_ram0_4_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 4)
     
    def test_core1_write_available_ram0_5_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 5)                                                    
     
    def test_core1_write_available_ram0_6_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 6)
         
    def test_core1_write_available_ram0_1KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x400)
     
    def test_core1_write_available_ram0_1KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x401)                                                    
     
    def test_core1_write_available_ram0_2KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x800)
     
    def test_core1_write_available_ram0_2KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x801)                                                    
     
    def test_core1_write_available_ram0_4KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1000)
     
    def test_core1_write_available_ram0_4KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1001)                                                    
       
    def test_core1_write_half_of_available_ram0(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'halfOfAvailableMemory')
     
    def test_core1_write_all_of_available_ram0(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'allOfAvailableMemory')
 
    def test_core1_write_unaligned_ram0_address(self, bl):
        write_memory.write_unaligned_memory_address(bl, self.memType, self.memIndex)
     
    def test_core1_write_out_of_ram0(self, bl):
        write_memory.write_out_of_memory_range(bl, self.memType, self.memIndex)
     
    def test_core1_cumulative_write_ram0(self, bl):
        write_memory.cumulative_write(bl, self.memType, self.memIndex)
 
#############################################################################################################
# 
#                Write Flash 1 with core 0.
# 
############################################################################################################# 
class TestCore1WriteFlash1:
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
    
    def test_core1_write_available_flash1_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 1)
    
    def test_core1_write_available_flash1_2_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 2)                                                    
    
    def test_core1_write_available_flash1_3_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 3)
        
    def test_core1_write_available_flash1_4_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 4)
    
    def test_core1_write_available_flash1_5_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 5)                                                    
    
    def test_core1_write_available_flash1_6_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 6)
        
    def test_core1_write_available_flash1_1KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x400)
    
    def test_core1_write_available_flash1_1KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x401)                                                    
    
    def test_core1_write_available_flash1_2KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x800)
    
    def test_core1_write_available_flash1_2KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x801)                                                    
    
    def test_core1_write_available_flash1_4KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1000)
    
    def test_core1_write_available_flash1_4KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1001)                                                    
      
    def test_core1_write_half_of_available_flash1(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'halfOfAvailableMemory')
    
    def test_core1_write_all_of_available_flash1(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'allOfAvailableMemory')
    
    def test_core1_write_unaligned_flash1_address(self, bl):
        write_memory.write_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_write_out_of_flash1(self, bl):
        write_memory.write_out_of_memory_range(bl, self.memType, self.memIndex)
    
    def test_core1_cumulative_write_flash1(self, bl):
        write_memory.cumulative_write(bl, self.memType, self.memIndex)
 
 #############################################################################################################
 #
 #                Write SRAM 1 with core 0.
 #
 ############################################################################################################# 
class TestCore1WriteSram1:
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
     
    def test_core1_write_available_ram1_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 1)
    
    def test_core1_write_available_ram1_2_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 2)                                                    
    
    def test_core1_write_available_ram1_3_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 3)
        
    def test_core1_write_available_ram1_4_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 4)
    
    def test_core1_write_available_ram1_5_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 5)                                                    
    
    def test_core1_write_available_ram1_6_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 6)
        
    def test_core1_write_available_ram1_1KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x400)
    
    def test_core1_write_available_ram1_1KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x401)                                                    
    
    def test_core1_write_available_ram1_2KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x800)
    
    def test_core1_write_available_ram1_2KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x801)                                                    
    
    def test_core1_write_available_ram1_4KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1000)
    
    def test_core1_write_available_ram1_4KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1001)                                                    
      
    def test_core1_write_half_of_available_ram1(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'halfOfAvailableMemory')
    
    def test_core1_write_all_of_available_ram1(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'allOfAvailableMemory')

    def test_core1_write_unaligned_ram1_address(self, bl):
        write_memory.write_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_write_out_of_ram1(self, bl):
        write_memory.write_out_of_memory_range(bl, self.memType, self.memIndex)
    
    def test_core1_cumulative_write_ram1(self, bl):
        write_memory.cumulative_write(bl, self.memType, self.memIndex)
 
 #############################################################################################################
 #
 #                Write SRAM 2 with core 0.
 #
 ############################################################################################################# 
class TestCore1WriteSram2:
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
     
    def test_core1_write_available_ram2_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 1)
    
    def test_core1_write_available_ram2_2_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 2)                                                    
    
    def test_core1_write_available_ram2_3_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 3)
        
    def test_core1_write_available_ram2_4_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 4)
    
    def test_core1_write_available_ram2_5_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 5)                                                    
    
    def test_core1_write_available_ram2_6_Bytes(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 6)
        
    def test_core1_write_available_ram2_1KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x400)
    
    def test_core1_write_available_ram2_1KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x401)                                                    
    
    def test_core1_write_available_ram2_2KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x800)
    
    def test_core1_write_available_ram2_2KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x801)                                                    
    
    def test_core1_write_available_ram2_4KB(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1000)
    
    def test_core1_write_available_ram2_4KB_plus_1_Byte(self, bl):
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 0x1001)                                                    
      
    def test_core1_write_half_of_available_ram2(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'halfOfAvailableMemory')
    
    def test_core1_write_all_of_available_ram2(self, bl):   
        write_memory.write_start_of_available_memory_region(bl, self.memType, self.memIndex, 'allOfAvailableMemory')

    def test_core1_write_unaligned_ram2_address(self, bl):
        write_memory.write_unaligned_memory_address(bl, self.memType, self.memIndex)
    
    def test_core1_write_out_of_ram2(self, bl):
        write_memory.write_out_of_memory_range(bl, self.memType, self.memIndex)
    
    def test_core1_cumulative_write_ram2(self, bl):
        write_memory.cumulative_write(bl, self.memType, self.memIndex)
     
         
         
         