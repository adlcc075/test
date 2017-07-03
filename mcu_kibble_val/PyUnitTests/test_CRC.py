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
import time
import random

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util
from common import user_config_area
 
        
@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if tgt.isCRCCheckSupported == False:
        pytest.skip('%s does not support CRC.' %(tgt.targetName))
         
         
#define a function to create data for writing 
def create_data_file(bl,
                    FirstFourByte = 0xffffffff,
                    SecondFourByte= 0xffffffff):
    Data = bytearray()
    # Fill the specific data to the BCA data array.
    user_config_area.fill_data_32bits(Data, FirstFourByte)
    user_config_area.fill_data_32bits(Data, SecondFourByte)
    # Write the data  to the data.dat file
    filePath = os.path.join(bl.vectorsDir, 'Data.dat')
    fileObj = open(filePath, 'w+')
    fileObj.write(Data)
    fileObj.close()
    return filePath
 
     
class TestCRC:
     
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        status, results = bl.reset()
        time.sleep(3)
        assert status == bootloader.status.kStatus_Success
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)
         
    def test_CRC_check_invalid(self, bl):
    
            
        #1.Erase all and send command 8 to confirm whether the returned value is CheckInvalid
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckInvalid
          
        # 1.write crc field all 0xff  
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag=user_config_area.four_char_code('k', 'c', 'f', 'g'),
                                                       crcStartAddress =0xffffffff,
                                                       crcByteCount    =0xffffffff,
                                                       crcExpectedValue=0xffffffff)
           
           
        # 2. Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
   
        # 3. Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
   
        # get the crc status
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckInvalid
          
          
       
    def test_CRC_check_inactive(self, bl):
          
          
        #set tag and all the crc value are not all 0xff  
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag=user_config_area.four_char_code('k', 'c', 'f', 'g'),
                                                       crcStartAddress =0x00000000,
                                                       crcByteCount    =0x00000000,
                                                       crcExpectedValue=0x00000000)
          
        #  Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
   
        #  Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
   
          
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckInactive 
                           
    def test_CRC_check_out_of_range(self, bl):
          
       
        #case 1:BCA is invalid and CRC byte count is equal to zero
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag=user_config_area.four_char_code('k', 'c', 'f', 'g'),
                                                       crcStartAddress =0x00000000,
                                                       crcByteCount    =0x00000000,
                                                       crcExpectedValue=0xffffffff)
          
        #  Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
  
        #Create file which contains PC pointer
        filePath = create_data_file(bl,
                                  FirstFourByte  = 0xffffffff,
                                  SecondFourByte = 0x00000001,)
          
        #write PC pointer to Vector_table_address
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, filePath)
        assert status == bootloader.status.kStatus_Success
   
        #  Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
          
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckOutOfRange 
          
        #erase and reset
        common_util.erase_all_available_flash_unsecure(bl)
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success 
        time.sleep(3)
          
          
          
        #case 2:BCA is invalid and CRC address exceed 4G
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag=user_config_area.four_char_code('k', 'c', 'f', 'g'),
                                                       crcStartAddress =0xffffffff,
                                                       crcByteCount    =0x00000002,
                                                       crcExpectedValue=0xffffffff)
          
        #  Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
  
        #Create file which contains PC pointer
        filePath = create_data_file(bl,
                                  FirstFourByte  = 0xffffffff,
                                  SecondFourByte = 0x00000001,)
          
        #write PC pointer to Vector_table_address
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, filePath)
        assert status == bootloader.status.kStatus_Success
   
        #  Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
          
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckOutOfRange 
          
        #erase and reset
        common_util.erase_all_available_flash_unsecure(bl)
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success 
        time.sleep(3)
          
          
          
        #case 3:BCA is invalid and CRC address exceed flash size
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag=user_config_area.four_char_code('k', 'c', 'f', 'g'),
                                                       crcStartAddress =0x10000000,
                                                       crcByteCount    =0x00000002,
                                                       crcExpectedValue=0xffffffff)
          
        #  Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
  
        #Create file which contains PC pointer
        filePath = create_data_file(bl,
                                  FirstFourByte  = 0xffffffff,
                                  SecondFourByte = 0x00000001,)
          
        #write PC pointer to Vector_table_address
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, filePath)
        assert status == bootloader.status.kStatus_Success
   
        #  Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
          
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckOutOfRange 
         
 
         
         
    def test_CRC_check_failed(self, bl):
         
        #BCA is invalid and everything is right but CRC excepted value is wrong
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag=user_config_area.four_char_code('k', 'c', 'f', 'g'),
                                                       crcStartAddress =bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x400,
                                                       crcByteCount    =0x00000008,
                                                       crcExpectedValue=0x12000000)
         
        #  Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
 
        #create data for crc checking 
        filePath = create_data_file(bl,
                                  FirstFourByte  = 0x88776655,
                                  SecondFourByte = 0x44332211,)
         
        #write data to vector_address + 0x400 for CRC checking
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x400, filePath)
        assert status == bootloader.status.kStatus_Success
         
         
        #Create file which contains PC pointer
        filePath = create_data_file(bl,
                                  FirstFourByte  = 0xffffffff,
                                  SecondFourByte = bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x400,)
         
        #write PC pointer to Vector_table_address
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, filePath)
        assert status == bootloader.status.kStatus_Success
  
        #  Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
         
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckFailed 
         
    def test_CRC_check_pass(self, bl):
         
        #BCA is invalid and everything is right 
        #CRC exceped calue is 0xE9833E8B
        #data for CRC checking is 0xcd
        #CRC check byte is 0x1
        #CRC check address is Vector_table_address + 0x400
     
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag=user_config_area.four_char_code('k', 'c', 'f', 'g'),
                                                       crcStartAddress = bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x400,
                                                       crcByteCount    = 0x00000001,
                                                       crcExpectedValue= 0xE9833E8B)
         
        #  Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
 
        #create data for crc checking 
        filePath = create_data_file(bl,
                                  FirstFourByte  = 0xffffffcd,
                                  SecondFourByte = 0xffffffff,)
         
        #write data to vector_address + 0x400 for CRC checking
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x400, filePath)
        assert status == bootloader.status.kStatus_Success
         
         
        #Create file which contains PC pointer
        filePath = create_data_file(bl,
                                  FirstFourByte  = 0xffffffff,
                                  SecondFourByte = bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x400)
         
        #write PC pointer to Vector_table_address
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, filePath)
        assert status == bootloader.status.kStatus_Success
  
        #  Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
         
        status, results = bl.get_property(8)
        assert results[0] == bootloader.status.kStatus_AppCrcCheckPassed