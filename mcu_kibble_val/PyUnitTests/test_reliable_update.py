#! /usr/bin/env python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
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
import struct

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from common import common_util
from common import crc32

# ##
# # @brief Skip all the tests in this module if the target does not support
# #        the ReliableUpdate command.
# #
# @pytest.fixture(scope='module', autouse=True)
# def skipif(tgt, request):
#     if not tgt.supportHwReliableUpdate:
#         pytest.skip('%s does not support HW reliable update.' % (tgt.cpu))    


def generate_demo(bl, isBcaOpen, isCrcOpen, isCrcRight):
    elfFile, hexFile, binFile = common_util.get_led_demo_path(bl)
    vectorAddress = bl.target.BL_APP_VECTOR_TABLE_ADDRESS
    #create a bytearray to store the bca and CRC configuration
    byte = bytearray()
    if isBcaOpen:
        byte.append(ord('k'))
        byte.append(ord('c'))
        byte.append(ord('f'))
        byte.append(ord('g'))
    else:
        byte.append(ord('k'))
        byte.append(ord('c'))
        byte.append(ord('f'))
        byte.append(ord('a'))
    #crc start address
    if isCrcOpen:
        byte.append( vectorAddress & 0x000000ff)
        byte.append((vectorAddress & 0x0000ff00)>>8)
        byte.append((vectorAddress & 0x00ff0000)>>16)
        byte.append((vectorAddress & 0xff000000)>>24)
    else:
        byte.append(0xff)
        byte.append(0xff)
        byte.append(0xff)
        byte.append(0xff)
    #crc count to be calculated
    byte.append(0x4)
    byte.append(0x00)
    byte.append(0x00)
    byte.append(0x00)
    #create a list to store the first 4 bytes value of APP, then use crc algorithm to calculate the 4 bytes             
    appDataArray = range(0,4)
    with open(binFile, 'rb+') as fileObj:
        for i in range(0,4):
            data = fileObj.read(int(1))
            appDataArray[i] = (ord(data))
        #value, = struct.unpack('L', data)
        print appDataArray
        crcResult = crc32.ComputeCRC32().calculate(0xffffffff, appDataArray[0:4])
        print crcResult
    #crc expected result
        if isCrcRight:
            byte.append(crcResult & 0x000000ff)
            byte.append((crcResult & 0x0000ff00)>>8)
            byte.append((crcResult & 0x00ff0000)>>16)
            byte.append((crcResult & 0xff000000)>>24)
        else:
            byte.append(0xff) 
            byte.append(0xff)
            byte.append(0xff)
            byte.append(0xff)
        #get the BCA loaction, then modify BCA & CRC configuration of the app
        fileObj.seek(0x3c0,0) 
        fileObj.write(byte)
        fileObj.close() 
    return binFile
    

# def FillDataInTestAppArray(array, loc, data):
#     array[loc] = data & 0xFF
#     array[loc+1] = (data & 0xFF00) >> 8
#     array[loc+2] = (data & 0xFF0000) >> 16
#     array[loc+3] = (data & 0xFF000000) >> 24   
#  
# # get SP for app     
# def get_SP(bl):
#     startAddress, length = common_util.get_start_address_and_length(bl, 1, 'AllMemory', 'ram', 'StartOfMemory') 
#     SP = startAddress + length -8 
#     print "RAM start:",hex(startAddress)
#     print "RAM size", hex(length)
#     print "SP:", hex(SP)
#     return SP
#  
#  #get PC for app
# def get_PC(bl):
#     startAddress, length = common_util.get_start_address_and_length(bl, 1, 'AllMemory', 'flash', 'StartOfMemory') 
#     PC = bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x411
#     print "PC:", hex(PC)
#     return PC
#     
# # generate app for test reliable update
# def generateApp(bl, isBcaRight, isCrcOpen, isCrcRight):
#     appFilePath =  os.path.abspath(os.path.join(bl.vectorsDir, 'App_for_reliable_update.dat'))
#     appDataArray = range(0, 1060)
#     for i in range(0, 1060):
#         appDataArray[i] = 0xff   
#  
#     AppSP = get_SP(bl)
#     FillDataInTestAppArray(appDataArray, 0, AppSP)  
#     #AppPC = 0xa411;
#     AppPC = get_PC(bl)
#     FillDataInTestAppArray(appDataArray, 4, AppPC)
#     bcaTag = 0x6766636b
#     if not isBcaRight:
#         bcaTag = 0x67666367
#     FillDataInTestAppArray(appDataArray, 960, bcaTag)
#     #CRC start address is app_vector_table, reliable update require this value must be equal with app_vector_table
#     if isCrcOpen:
#         crcStart = bl.target.BL_APP_VECTOR_TABLE_ADDRESS
#         FillDataInTestAppArray(appDataArray, 964, crcStart)
#     #cal 4 bytes
#     crcBytes = 4
#     FillDataInTestAppArray(appDataArray, 968, crcBytes)
#     #CRC calculate first 4 bytes of app , reliable update CRC cal start address must be the same as app_vector_table 
#     crcResult = crc32.ComputeCRC32().calculate(0xffffffff, appDataArray[0:4])
#     if not isCrcRight:
#         crcResult = crcResult + 1   
#     FillDataInTestAppArray(appDataArray, 972, crcResult)
#     #app content is hello at 0xa411
#     appContent = 0x6c6c6568
#     FillDataInTestAppArray(appDataArray, 1041, appContent)
#     appContent = 0x6f
#     FillDataInTestAppArray(appDataArray, 1045, appContent)
#      
#     with open(appFilePath, 'wb') as fileObj:
#         for i in range (len(appDataArray)):
#             fileObj.write(chr(appDataArray[i]))
#         fileObj.close() 
#                  
#     return appFilePath
  
def get_backup_address(bl):
    startAddress, length = common_util.get_start_address_and_length(bl, 1, 'AllMemory', 'flash', 'StartOfMemory') 
    backUpAddress = length/2 + bl.target.BL_APP_VECTOR_TABLE_ADDRESS
    print 'backUpAddress:', hex(backUpAddress)
    return backUpAddress 
 
def get_indicator_address(bl, status):
    startAddress      = common_util.get_memory_start_address(bl, 'flash')
    totalSize         = common_util.get_memory_total_size(bl, 'flash')
    sectorSize        = common_util.get_flash_sector_size(bl)
    if status:
        #address for reliable update
        address = startAddress + totalSize/2 - sectorSize
        return address
    else:
        #wrong address for test swap indicator address invalid
        address = startAddress + totalSize/2 - sectorSize * 2
        return address
 
def verify(bl):
    #read first 4 bytes from app vector table
    readFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'tmp_read.dat'))   
    status, results = bl.read_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, 4, readFilePath)
    assert status == bootloader.status.kStatus_Success
    #compare with 0xffffffff to confirm whether reliable update has been executed successfully  
    readBytesArray = None    
    with open(readFilePath, 'rb') as file:
        readBytesArray = file.read(int (4))
        file.close
    #unpack the 4 bytes value from readfile 
    value, = struct.unpack('L', readBytesArray)
    assert value != 0xffffffff
        
class TestHwReliableUpdate:
    @pytest.fixture(autouse=True)
    def setup(self, bl, request):
        if not bl.target.isHwReliableUpdateSupported:
            pytest.skip('%s does not support HW reliable update.' % (bl.target.targetName)) 
        common_util.reset_with_check(bl)
        #erase app vector region
        status, result = bl.flash_erase_region(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, 0x1000)
        assert status == bootloader.status.kStatus_Success
        def teardown():
            pass
        request.addfinalizer(teardown)
     
    def test_Hw_reliable_swap_system_not_ready(self, bl):
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success
        #reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
              
        status, results = bl.get_property(bootloader.properties.kPropertyTag_ReliableUpdate)
        assert status == bootloader.status.kStatus_Success
        assert results[0] == bootloader.status.kStatus_ReliableUpdateSwapSystemNotReady  
          
    def test_Hw_reliable_stil_in_main_region(self, bl):
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success 
        #generate right app for testing reliable update
        appFile = generate_demo(bl, True, True, True)
        status,result = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)  
        #reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(1) 
        #check returned value
        status, results = bl.get_property(bootloader.properties.kPropertyTag_ReliableUpdate)
        assert status == bootloader.status.kStatus_Success
        assert results[0] == bootloader.status.kStatus_ReliableUpdateStillInMainApplication     
            
    def test_Hw_reliable_update_inactive(self, bl):
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success
        #app with wrong BCA
        appFile = generate_demo(bl, False, True, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #reliable update
        indicatorAddress = get_indicator_address(bl, True)
        status, results = bl.reliable_update(indicatorAddress)
        assert status == bootloader.status.kStatus_ReliableUpdateInactive 
               
    def test_Hw_reliable_update_invalid(self, bl):   
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success     
        #app with right BCA but don't support checking CRC
        appFile = generate_demo(bl, True, False, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)  
        #reliable update
        indicatorAddress = get_indicator_address(bl, True)
        status, results = bl.reliable_update(indicatorAddress)
        assert status == bootloader.status.kStatus_ReliableUpdateBackupApplicationInvalid
                     
    def test_Hw_reliable_update_success(self, bl):  
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success
        #generate right app for testing reliable update
        appFile = generate_demo(bl, True, True, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #reliable update
        indicatorAddress = get_indicator_address(bl, True)
        status, results = bl.reliable_update(indicatorAddress)
        assert status == bootloader.status.kStatus_ReliableUpdateSuccess
             
    def test_Hw_reliable_swap_indicator_address_invalid(self, bl):  
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success  
        #generate right app for testing reliable update
        appFile = generate_demo(bl, True, True, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #reliable update
        indicatorAddress = get_indicator_address(bl, False)
        status, results = bl.reliable_update(indicatorAddress)
        assert status == bootloader.status.kStatus_ReliableUpdateSwapIndicatorAddressInvalid
             
             
    def test_Hw_reliable_update_reset_method(self, bl):  
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success  
        #generate right app for testing reliable update
        appFile = generate_demo(bl, True, True, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #reset twice to execute reliable update
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #to check whether reliable update has been executed
        verify(bl)
                 
         
class TestSwReliableUpdate:
    @pytest.fixture(autouse=True)
    def setup(self, bl, request):
        if not bl.target.isSwReliableUpdateSupported:
             pytest.skip('%s does not support SW reliable update.' % (bl.target.targetName)) 
        common_util.reset_with_check(bl)
        #erase app vector region
        status, result = bl.flash_erase_region(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, 0x1000)
        assert status == bootloader.status.kStatus_Success    
            
    def test_Sw_reliable_update_inactive(self, bl):
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success
        #app with wrong BCA
        appFile = generate_demo(bl, False, True, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #reliable update
        status, results = bl.reliable_update(backUpAddress)
        assert status == bootloader.status.kStatus_ReliableUpdateInactive 
              
    def test_Sw_reliable_update_invalid(self, bl):   
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success     
        #app with right BCA but don't support checking CRC
        appFile = generate_demo(bl, True, False, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)  
        #reliable update
        indicatorAddress = get_indicator_address(bl, True)
        status, results = bl.reliable_update(backUpAddress)
        assert status == bootloader.status.kStatus_ReliableUpdateBackupApplicationInvalid
                     
    def test_Sw_reliable_update_success(self, bl):  
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(backUpAddress, 0x1000)
        assert status == bootloader.status.kStatus_Success
        #generate right app for testing reliable update
        appFile = generate_demo(bl, True, True, True)
        status,result = bl.write_memory(backUpAddress, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #reliable update
        status, results = bl.reliable_update(backUpAddress)
        assert status == bootloader.status.kStatus_ReliableUpdateSuccess
             
             
    def test_Sw_reliable_update_reset_method(self, bl):  
        #erase backup region 
        backUpAddress = get_backup_address(bl)
        status, result = bl.flash_erase_region(bl.target.backUpAppStart, 0x1000)
        assert status == bootloader.status.kStatus_Success  
        #generate right app for testing reliable update
        appFile = generate_demo(bl, True, True, True)
        status,result = bl.write_memory(bl.target.backUpAppStart, appFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #to check whether reliable update has been executed
        verify(bl)

        

        
        