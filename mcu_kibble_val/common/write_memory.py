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
from common.SBFile import SBFile

def write_start_of_available_memory_region(bl, memType, memIndex, bytesNumber, usingSbComamnd=False):
    (availableMemStartAddress, availableMemEndAddress, availableMemSize) = common_util.get_available_memory_region(bl, memType, memIndex)
    if(availableMemSize == 0): # K3S TO1.0, all the M4 ITCM SRAM regions are reserved.
        print("available%s%dSize = %d Bytes." %(memType.capitalize(), memIndex, availableMemSize))
        print("No available region for %s%d!"%(memType.capitalize(), memIndex))
        return
    
    if(bytesNumber == 'halfOfAvailableMemory'):
        bytesNumber = availableMemSize / 2
    elif(bytesNumber == 'allOfAvailableMemory'):
        bytesNumber = availableMemSize    
    
    if(bytesNumber <= availableMemSize):
        writeStartAddress = availableMemStartAddress
        writeBytesLength = bytesNumber
        writeEndAddress = writeStartAddress + writeBytesLength
        binFilePath = common_util.generate_random_data_file(bl, writeStartAddress, writeBytesLength)
        if(usingSbComamnd == True):
            bdContent = SBFile.bd_content_init(bl, needSign=False)
            bdContent = SBFile.bd_content_update_for_write_memory(bdContent, binFilePath, writeStartAddress, writeEndAddress)
            bdFile = SBFile.bd_content_finish(bl, bdContent)
            sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
            status, results = bl.receive_sb_file(sbFile)
        else:       
            status, results = bl.write_memory(writeStartAddress, binFilePath)  
        assert status == bootloader.status.kStatus_Success
        verifyWriteResult = common_util.verify_write_memory(bl, writeStartAddress, binFilePath)
        assert verifyWriteResult == True            
    else:
        print("writeLength = %d Bytes > available%s%dSize = %d Bytes." %(bytesNumber, memType.capitalize(), memIndex, availableMemSize))

def write_reserved_memory_region(bl, memType, usingSbComamnd=False):
    (reservedMemStartAddress, reservedMemEndAddress, reservedMemSize) = common_util.get_reserved_memory_region(bl, memType)
    if(reservedMemSize == 0):  # ROM has no reserved flash region
        print('No reserved region for %s memory.' %(memType.upper()))
        return
    else:
        writeStartAddress = reservedMemStartAddress
        writeBytesLength = reservedMemSize
        writeEndAddress = writeStartAddress + writeBytesLength
        binFilePath = common_util.generate_random_data_file(bl, writeStartAddress, writeBytesLength)
        if(usingSbComamnd == True):
            bdContent = SBFile.bd_content_init(bl, needSign=False)
            bdContent = SBFile.bd_content_update_for_write_memory(bdContent, binFilePath, writeStartAddress, writeEndAddress)
            bdFile = SBFile.bd_content_finish(bl, bdContent)
            sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
            status, results = bl.receive_sb_file(sbFile)
        else:       
            status, results = bl.write_memory(writeStartAddress, binFilePath)
        assert status == bootloader.status.kStatusMemoryRangeInvalid
        
def write_unaligned_memory_address(bl, memType, memIndex, usingSbComamnd=False):
    (availableMemStartAddress, availableMemEndAddress, availableMemSize) = common_util.get_available_memory_region(bl, memType, memIndex)
    if(availableMemSize == 0): # K3S TO1.0, all the M4 ITCM SRAM regions are reserved.
        print("available%s%dSize = %d Bytes." %(memType.capitalize(), memIndex, availableMemSize))
        print("No need to write the unagined memory address for %s%d!"%(memType.capitalize(), memIndex))
        return
    
    writeBytesLength = 0x10
    for i in range(1, bl.target.programAlignmentSize):
        writeStartAddress = availableMemStartAddress + i # unaligned address
        writeEndAddress = writeStartAddress + writeBytesLength
        # If not in the available region, just quit the loop
        if(writeStartAddress + writeBytesLength > availableMemEndAddress):
            break
        # If in the available region
        else:
            binFilePath = common_util.generate_random_data_file(bl, writeStartAddress, writeBytesLength)
            if(usingSbComamnd == True):
                bdContent = SBFile.bd_content_init(bl, needSign=False)
                bdContent = SBFile.bd_content_update_for_write_memory(bdContent, binFilePath, writeStartAddress, writeEndAddress)
                bdFile = SBFile.bd_content_finish(bl, bdContent)
                sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
                status, results = bl.receive_sb_file(sbFile)
            else:       
                status, results = bl.write_memory(writeStartAddress, binFilePath)

            # 1. For SRAM, should return success
            if(memType == 'ram'):
                assert status == bootloader.status.kStatus_Success
                verifyWriteResult = common_util.verify_write_memory(bl, writeStartAddress, binFilePath)
                assert verifyWriteResult == True
            elif(memType == 'flash'):
                assert status == bootloader.status.kStatus_FlashAlignmentError

def write_out_of_memory_range(bl, memType, memIndex, usingSbComamnd=False):
    memStartAddress, memEndAddress = common_util.get_memory_start_end_address(bl, memType, memIndex)
    writeStartAddress = memEndAddress + 1
    writeBytesLength = 0x400
    writeEndAddress = writeStartAddress + writeBytesLength
    binFilePath = common_util.generate_random_data_file(bl, writeStartAddress, writeBytesLength)
    if(usingSbComamnd == True):
        bdContent = SBFile.bd_content_init(bl, needSign=False)
        bdContent = SBFile.bd_content_update_for_write_memory(bdContent, binFilePath, writeStartAddress, writeEndAddress)
        bdFile = SBFile.bd_content_finish(bl, bdContent)
        sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
        status, results = bl.receive_sb_file(sbFile)
    else:       
        status, results = bl.write_memory(writeStartAddress, binFilePath)
    assert status == bootloader.status.kStatusMemoryRangeInvalid

def cumulative_write(bl, memType, memIndex, usingSbComamnd=False):
    (availableMemStartAddress, availableMemEndAddress, availableMemSize) = common_util.get_available_memory_region(bl, memType, memIndex)
    if(availableMemSize == 0): # K3S TO1.0, all the M4 ITCM SRAM regions are reserved.
        print("available%s%dSize = %d Bytes." %(memType.capitalize(), memIndex, availableMemSize))
        print("No need to test cumulative write for %s%d!"%(memType.capitalize(), memIndex))
        return
    else:
        writeStartAddress = availableMemStartAddress
        writeBytesLength = 0x400
        writeEndAddress = writeStartAddress + writeBytesLength
        # Generate two random bin files.
        binFilePath1 = common_util.generate_random_data_file(bl, writeStartAddress, writeBytesLength)
        binFilePath2 = common_util.generate_random_data_file(bl, writeStartAddress, writeBytesLength)
        if(usingSbComamnd == True):
            bdContent = SBFile.bd_content_init(bl, needSign=False)
            bdContent = SBFile.bd_content_update_for_write_memory(bdContent, binFilePath1, writeStartAddress, writeEndAddress)
            bdContent = SBFile.bd_content_update_for_write_memory(bdContent, binFilePath2, writeStartAddress, writeEndAddress)
            bdFile = SBFile.bd_content_finish(bl, bdContent)
            sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
            status, results = bl.receive_sb_file(sbFile)
        else:
            status, results = bl.write_memory(writeStartAddress, binFilePath1)
            assert status == bootloader.status.kStatus_Success
            print('\nCumulative write %s:'%memType),
            status, results = bl.write_memory(writeStartAddress, binFilePath2)
        if(memType == 'ram'):
            assert status == bootloader.status.kStatus_Success
        elif(memType == 'flash'):
            assert ( (status == bootloader.status.kStatus_FlashCommandFailure) or 
                     (status == bootloader.status.kStatusMemoryCumulativeWrite) )


# EOF