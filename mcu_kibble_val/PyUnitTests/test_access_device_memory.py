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

import sys, os
import array
import random
import time
import struct
import pytest

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util


# @brief Skip all the tests in this module if the target does not support
#        access device memory.
@pytest.fixture(scope='module', autouse=True)
def skipif(tgt, request):
    if not tgt.deviceMemoryAccessable:
        pytest.skip('%s does not support access device memory.' % (tgt.cpu))

# #################################################
# #8\16\32 bit register
# kSIM_AddressFor8BitModuleClockGate = 0x40048034  # SIM_SCGC4
# kClockGatePosFor8BitModule = 10                  # UART0
# kAccessAddressFor8BitModuleStart = 0x4006A002  # UART0_BDH
# kAccessLengthFor8BitModule = 1
# 
# kSIM_AddressFor16BitModuleClockGate = 0x4004803c # SIM_SCGC6
# kClockGatePosFor16BitModule = 1 # DMA
# kAccessAddressFor16BitModuleStart = 0x40009014
# kAccessLengthFor16BitModule = 2
# 
# kSIM_AddressFor32BitModuleClockGate = 0x4004803c # SIM_SCGC6
# kClockGatePosFor32BitModule = 1    #DMA
# kAccessAddressFor32BitModuleStart = 0x40009028 # DMA_TCD1_NBYTES_MLNO
# kAccessLengthFor32BitModule = 8

#################################################
# @brief FTFx moudle register address
kFTFx_STAT      = 0x40020000
kFTFx_FCNFG     = 0x40020001
kFTFx_FSEC      = 0x40020002
KFTFx_FOPT      = 0x40020003
kFTFx_FCCOB3    = 0x40020004
kFTFx_FCCOB2    = 0x40020005
kFTFx_FCCOB1    = 0x40020006
kFTFx_FCCOB0    = 0x40020007
kFTFx_FCCOB7    = 0x40020008
kFTFx_FCCOB6    = 0x40020009
kFTFx_FCCOB5    = 0x4002000a
kFTFx_FCCOB4    = 0x4002000b
kFTFx_FCCOBB    = 0x4002000c
kFTFx_FCCOBA    = 0x4002000d
kFTFx_FCCOB9    = 0x4002000e
kFTFx_FCCOB8    = 0x4002000f

# @brief FTFx commands
kFTFx_Command_ERASE_SECTOR = 0x09
kFTFx_Command_PROGRAM_LONGWORD = 0x06
kFTFx_Command_PROGRAM_PHASE = 0x07



# @brief Unify temporary file used in test cases
def file_path(bl):
    return os.path.abspath(os.path.join(bl.vectorsDir, 'Data.dat'))

def verify_memory(bl, address, length, value):
    #create a file named 'verify.dat' to receive the content from read_memory
    file = os.path.abspath(os.path.join(bl.vectorsDir, 'verify.dat'))
    expectedBytes = array.array('c', [chr(value)]) * length
    #read content 
    status, result = bl.read_memory(address, len(expectedBytes), file)
    assert status == bootloader.status.kStatus_Success
    #compare the returned value with the expected value
    bootloaderBytes = open(file,'rb').read()
    return bootloaderBytes == expectedBytes.tostring()

# @brief Read register value from target, length should be 1, 2, or 4
def read_register(bl, address, length):
    kFilePath = file_path(bl)
    status, results = bl.read_memory(address, length, kFilePath)
    assert status == bootloader.status.kStatus_Success
    #get value from file
    readBackBytes = None
    with open (kFilePath, 'rb') as file:
        readBackBytes = file.read(int(length))
        file.close()   
    #chose unpack type 
    if length == 1:
        format = 'B'
    elif length == 2:
        format = 'H'
    elif length == 4:
        format = 'L'
    #unpack the value from file so that we can get the value from memory
    assert readBackBytes != None
    value, = struct.unpack(format, readBackBytes)
    return value

# @brief Write certian value to register, length should be 1, 2, or 4
def write_register(bl, address, value, length):
    #chose pack type
    if length == 1:
        format = 'B'
    elif length == 2:
        format = 'H'
    elif length == 4:
        format = 'L'
    writeBackBytes = struct.pack(format, value)
    #write packed value to file
    kFilePath = file_path(bl)
    if os.path.exists(kFilePath):
        os.remove(kFilePath)    
        
    with open (kFilePath, 'wb') as file:
        file.write(writeBackBytes)
        file.close()
    #write value to the register  
    status, results = bl.write_memory(address, kFilePath)
    assert status == bootloader.status.kStatus_Success
    
# @brief Control clock gate of specified IP module
def control_clock_gate(bl, address, position, enable=True):
    lastRegVal = read_register(bl, address, 4)
    #enable means we should set the bit as 1 so that we can access the register
    if enable:
        expectedRegVal = lastRegVal | (1 << position)
    else:
        expectedRegVal = lastRegVal & (~(1 << position))
    
    write_register(bl, address, expectedRegVal, 4)
 
# @brief Verify Write/Read device memory command
def verify_access_device_memory(bl, startAddress, length, isRandom=True, fillValue = 0, alignedBit = 8):
    writeFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'write_data_to_memory.bin'))
    readFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'read_data_from_memory.bin'))
    
    writeBytesArray = bytearray(length)
    if not isRandom:
        if alignedBit == 8:
            for i in range (0, length):
                writeBytesArray[i] = fillValue & 0xFF
            pass
        elif alignedBit == 16:
            assert length & 2 == 0
            tmparray = struct.pack('H', fillValue & 0xFFFF)
            i = 0
            while i < length:
                writeBytesArray[i] = temparray[0]
                writeBytesArray[i+1] = temparray[1]
                i = i+2
        elif alignedBit == 32:
            assert length & 4 == 0
            tmparray = struct.pack('I', fillValue & 0xFFFF)
            i = 0
            while i < length:
                writeBytesArray[i] = tmparray[0]
                writeBytesArray[i+1] = tmparray[1]
                writeBytesArray[i+2] = tmparray[2]
                writeBytesArray[i+3] = tmparray[3]
                i = i+4
    else:
        for i in range (0, length):
            writeBytesArray[i] = random.randint(0, 65535) & 0xFF
        
    if os.path.exists(writeFilePath):
        os.remove(writeFilePath)
        
    with open(writeFilePath, 'wb') as file:
        file.write(writeBytesArray)
        file.close()
            
    status, results = bl.write_memory(startAddress, writeFilePath)
    assert status == bootloader.status.kStatus_Success
        
    status, results = bl.read_memory(startAddress, length, readFilePath)
    assert status == bootloader.status.kStatus_Success
        
    readBytesArray = None
        
    with open(readFilePath, 'rb') as file:
        readBytesArray = file.read(int (length))
        file.close
        
    assert readBytesArray == writeBytesArray
             
# @brief Flash command sequence, start and monitor status of flash operation            
def flash_command_sequence(bl):
    # clear RDCOLERR & ACCERR & FPVIOL flag in flash status register
    expectedRegVal = 0x70
    write_register(bl, kFTFx_STAT, expectedRegVal, 1)
    # clear CCIF bit
    expectedRegVal = 0x80
    write_register(bl, kFTFx_STAT, expectedRegVal, 1)
    
    isBusy = True
    while isBusy:
        currentRegVal = read_register(bl, kFTFx_STAT, 1)
        if (currentRegVal & 0x80) > 0:
            isBusy = False
    #read STAT to confirm the command has been executed successfully
    currentRegVal = read_register(bl, kFTFx_STAT, 1)
    assert currentRegVal & 0x70 == 0
    
# @brief Erase specified flash region
def flash_erase_one_sector(bl, startAddress):
    # pass address
    expectedRegVal = startAddress
    write_register(bl, kFTFx_FCCOB3, expectedRegVal, 4)
    # pass command
    expectedRegVal = kFTFx_Command_ERASE_SECTOR
    write_register(bl, kFTFx_FCCOB0, expectedRegVal, 1)
    # excute command
    flash_command_sequence(bl)
         
# @brief Fill specified flash region
def flash_fill(bl, startAddress, pattern, length):
    
    start = startAddress
    endAddress = startAddress + length - 1
    alignmentSize = bl.target.programAlignmentSize

    while (start < endAddress):
        # pass address
        expectedRegVal = start
        write_register(bl, kFTFx_FCCOB3, expectedRegVal, 4)
        
        # pass data
        expectedRegVal = pattern
        write_register(bl, kFTFx_FCCOB7, expectedRegVal, 4)
        if (alignmentSize == 4):
            # send 
            expectedRegVal = kFTFx_Command_PROGRAM_LONGWORD
            write_register(bl, kFTFx_FCCOB0, expectedRegVal, 1)
        elif (alignmentSize == 8):
            expectedRegVal = pattern
            write_register(bl, kFTFx_FCCOBB, expectedRegVal, 4)
            expectedRegVal = kFTFx_Command_PROGRAM_PHASE
            write_register(bl, kFTFx_FCCOB0, expectedRegVal, 1)  
        else:
            assert False # This type of FTFx module is not supported.
        flash_command_sequence(bl)
        start = start + alignmentSize
          
##############################################################################
#
#        Test cases
#
###############################################################################    
 
# class TestAccess8BitRegisters:
#     @pytest.fixture(autouse=True)
#     def setup(self, bl, request):
#         if bl.target.kAccessLengthFor8BitModule == 0:
#             pytest.skip("This test cases is not supported!")        
#         control_clock_gate(bl, bl.target.kSIM_AddressFor8BitModuleClockGate, bl.target.kClockGatePosFor8BitModule, True)
#    
#         def teardown_method():
#             control_clock_gate(bl, bl.target.kSIM_AddressFor8BitModuleClockGate, bl.target.kClockGatePosFor8BitModule, False)
#         request.addfinalizer(teardown_method)
#          
#     def test_access_8bit_registers(self, bl):
#         verify_access_device_memory(bl, bl.target.kAccessAddressFor8BitModuleStart, bl.target.kAccessLengthFor8BitModule)
#              
#               
# class TestAccess16BitRegisters:
#     @pytest.fixture(autouse=True)
#     def setup(self, bl, request):
#         if bl.target.kAccessLengthFor16BitModule == 0:
#             pytest.skip("This test cases is not supported!")
#         control_clock_gate(bl, bl.target.kSIM_AddressFor16BitModuleClockGate, bl.target.kClockGatePosFor16BitModule, True)
#               
#         def teardown_method():
#             control_clock_gate(bl, bl.target.kSIM_AddressFor16BitModuleClockGate, bl.target.kClockGatePosFor16BitModule, False)
#         request.addfinalizer(teardown_method)
#               
#     def test_access_16bit_registers(self, bl):
#         verify_access_device_memory(bl, bl.target.kAccessAddressFor16BitModuleStart, bl.target.kAccessLengthFor16BitModule)
#          
#          
# class TestAccess32BitRegisters:
#     @pytest.fixture(autouse=True)
#     def setup(self, bl, request):
#         if bl.target.kAccessLengthFor32BitModule == 0:
#             pytest.skip("This test cases is not supported!")
#         control_clock_gate(bl, bl.target.kSIM_AddressFor32BitModuleClockGate, bl.target.kClockGatePosFor32BitModule, True)
#              
#         def teardown_method():
#             control_clock_gate(bl, bl.target.kSIM_AddressFor32BitModuleClockGate, bl.target.kClockGatePosFor32BitModule, False)
#         request.addfinalizer(teardown_method)
#          
#     def test_access_32bit_registers(self, bl):
#         verify_access_device_memory(bl, bl.target.kAccessAddressFor32BitModuleStart, bl.target.kAccessLengthFor32BitModule, False, 0x20000000, 32)
    
          
class TestAccessFTFxModule:
    @pytest.fixture(autouse=True)
    def setup(self, bl, request):
        if (bltest_config.projectName.lower() == 'rom_bootloader_tower') or \
           (bltest_config.projectName.lower() == 'rom_bootloader_freedom'):
            pytest.skip('Does not support flash_resident bootloader.' ) 
        common_util.reset_with_check(bl)
        def teardown_method():
            pass
        request.addfinalizer(teardown_method)
     
    def test_access_flash_erase_region(self, bl):
        #get sector size and the last sector start address of flash
        startAddress, length = common_util.get_start_address_and_length(bl, 1, 'Sectors', 'flash', 'EndOfMemory') 
        # erase the sector
        status, result = bl.flash_erase_region(startAddress, length)
        assert status == bootloader.status.kStatus_Success
        #fill 0x00 to one sector    
        status, result = bl.fill_memory(startAddress, length, 0x00)
        assert status == bootloader.status.kStatus_Success 
        # use FTFx module to reliaze erase one sector
        flash_erase_one_sector(bl, startAddress) 
        # verify the sector has been erased successfully
        time.sleep(1)
        verify_memory(bl, startAddress, length, 0xFF)
                  
       
    def test_access_flash_fill(self, bl):
        file = file_path(bl);
        #get sector size and the last sector start address of flash
        startAddress, length = common_util.get_start_address_and_length(bl, 1, 'Sectors', 'flash', 'EndOfMemory')   
        # erase the sector
        status, result = bl.flash_erase_region(startAddress, length)
        assert status == bootloader.status.kStatus_Success
                
        length = 0x20
        pattern = 0x30
        #use FTFX to fill data length 0x20 , content 0x20 to the memory
        flash_fill(bl, startAddress, pattern, length)
        # verify         
        time.sleep(1)
        verify_memory(bl, startAddress, length, pattern)
  
  
          
