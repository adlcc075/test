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

def erase_sectors_at_start_of_available_region(bl, memType, memIndex, sectorsNumber, usingSbComamnd=False):
    (availableMemStartAddress, availableMemEndAddress, availableMemSize) = common_util.get_available_memory_region(bl, memType, memIndex)
    if(availableMemSize == 0): # K3S TO1.0, all the M4 ITCM SRAM regions are reserved.
        print("available%s%dSize = %d Bytes." %(memType.capitalize(), memIndex, availableMemSize))
        print("No available region for %s%d!"%(memType.capitalize(), memIndex))
        return
    
    if(str(sectorsNumber).isdigit()):
        # Get flash sector size
        flashSectorSize = common_util.get_flash_sector_size(bl, memIndex)
        eraseBytes = sectorsNumber * flashSectorSize
    elif(sectorsNumber == 'halfOfAvailableMemory'):
        eraseBytes = availableMemSize / 2
    elif(sectorsNumber == 'allOfAvailableMemory'):
        eraseBytes = availableMemSize
    
    
    if(eraseBytes <= availableMemSize):
        eraseStartAddress = availableMemStartAddress
        eraseBytesLength = eraseBytes
        eraseEndAddress = eraseStartAddress + eraseBytesLength              
        if(memType == 'ram'):
            if(usingSbComamnd == True):
                bdContent = SBFile.bd_content_init(bl, needSign=False)
                bdContent = SBFile.bd_content_update_for_flash_erase_region(bdContent, eraseStartAddress, eraseEndAddress)
                bdFile = SBFile.bd_content_finish(bl, bdContent)
                sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
                status, results = bl.receive_sb_file(sbFile)
                assert status == bootloader.status.kStatus_FlashAddressError
            else:
                status, results = bl.flash_erase_region(eraseStartAddress, eraseBytesLength)
                assert status == bootloader.status.kStatus_FlashAddressError
        elif(memType == 'flash'):
            print("Program the %s%d:"%(memType.capitalize(), memIndex)),
            status, results = bl.fill_memory(availableMemStartAddress, eraseBytes, 0x12345678, 'word')  
            assert status == bootloader.status.kStatus_Success
            if(usingSbComamnd == True):
                bdContent = SBFile.bd_content_init(bl, needSign=False)
                bdContent = SBFile.bd_content_update_for_flash_erase_region(bdContent, eraseStartAddress, eraseEndAddress)
                bdFile = SBFile.bd_content_finish(bl, bdContent)
                sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
                print("Erase the programmed data with erase sb command:"),
                status, results = bl.receive_sb_file(sbFile)
                assert status == bootloader.status.kStatus_Success
            else:
                print("Erase the programmed data with flash-erase-region command:"),
                status, results = bl.flash_erase_region(eraseStartAddress, eraseBytesLength) 
                assert status == bootloader.status.kStatus_Success
            verifyEraseResult = common_util.verify_flash_erase(bl, eraseStartAddress, eraseBytesLength)
            assert verifyEraseResult == True            
    else:
        print("eraseBytes = %d Bytes > available%s%dSize = %d Bytes." %(eraseBytes, memType.capitalize(), memIndex, availableMemSize))

def erase_reserved_memory_region(bl, memType, usingSbComamnd=False):
    (reservedMemStartAddress, reservedMemEndAddress, reservedMemSize) = common_util.get_reserved_memory_region(bl, memType)
    if(reservedMemSize == 0):  # ROM has no reserved flash region
        print('No reserved region for %s memory.' %(memType.upper()))
        return
    else:
        eraseStartAddress = reservedMemStartAddress
        eraseBytesLength = reservedMemSize
        eraseEndAddress = eraseStartAddress + eraseBytesLength
        if(usingSbComamnd == True):
            bdContent = SBFile.bd_content_init(bl, needSign=False)
            bdContent = SBFile.bd_content_update_for_flash_erase_region(bdContent, eraseStartAddress, eraseEndAddress)
            bdFile = SBFile.bd_content_finish(bl, bdContent)
            sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
            status, results = bl.receive_sb_file(sbFile)
        else:
            status, results = bl.flash_erase_region(eraseStartAddress, eraseBytesLength)

        if(memType == 'ram'):
            assert status == bootloader.status.kStatus_FlashAddressError
        elif(memType == 'flash'):
            assert status == bootloader.status.kStatusMemoryRangeInvalid
        
def erase_unaligned_memory_address(bl, memType, memIndex, usingSbComamnd=False):
    (availableMemStartAddress, availableMemEndAddress, availableMemSize) = common_util.get_available_memory_region(bl, memType, memIndex)
    if(availableMemSize == 0): # K3S TO1.0, all the M4 ITCM SRAM regions are reserved.
        print("available%s%dSize = %d Bytes." %(memType.capitalize(), memIndex, availableMemSize))
        print("No need to erase the unagined memory address for %s%d!"%(memType.capitalize(), memIndex))
        return
    
    for i in range(1, bl.target.eraseAlignmentSize):
        eraseStartAddress = availableMemStartAddress + i # unaligned address
        eraseBytesLength = bl.target.eraseAlignmentSize
        eraseEndAddress = eraseStartAddress + eraseBytesLength
        # If not in the available region, just quit the loop
        if(eraseStartAddress + eraseBytesLength > availableMemEndAddress):
            break
        # If in the available region
        else:
            if(usingSbComamnd == True):
                bdContent = SBFile.bd_content_init(bl, needSign=False)
                bdContent = SBFile.bd_content_update_for_flash_erase_region(bdContent, eraseStartAddress, eraseEndAddress)
                bdFile = SBFile.bd_content_finish(bl, bdContent)
                sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
                status, results = bl.receive_sb_file(sbFile)
            else:
                status, results = bl.flash_erase_region(eraseStartAddress, eraseBytesLength)

            if(memType == 'ram'):
                assert status == bootloader.status.kStatus_FlashAddressError
            elif(memType == 'flash'):
                assert status == bootloader.status.kStatus_FlashAlignmentError

def erase_unaligned_bytes_length(bl, memType, memIndex, usingSbComamnd=False):
    (availableMemStartAddress, availableMemEndAddress, availableMemSize) = common_util.get_available_memory_region(bl, memType, memIndex)
    if(availableMemSize == 0): # K3S TO1.0, all the M4 ITCM SRAM regions are reserved.
        print("available%s%dSize = %d Bytes." %(memType.capitalize(), memIndex, availableMemSize))
        print("No need to erase the unagined memory address for %s%d!"%(memType.capitalize(), memIndex))
        return
    
    for i in range(1, bl.target.eraseAlignmentSize):
        eraseStartAddress = availableMemStartAddress 
        eraseBytesLength = i     # unaligned byte length
        eraseEndAddress = eraseStartAddress + eraseBytesLength
      
        # If not in the available region, just quit the loop
        if(eraseStartAddress + eraseBytesLength > availableMemEndAddress):
            break
        # If in the available region
        else:
            if(usingSbComamnd == True):
                bdContent = SBFile.bd_content_init(bl, needSign=False)
                bdContent = SBFile.bd_content_update_for_flash_erase_region(bdContent, eraseStartAddress, eraseEndAddress)
                bdFile = SBFile.bd_content_finish(bl, bdContent)
                sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
                status, results = bl.receive_sb_file(sbFile)
            else:
                status, results = bl.flash_erase_region(eraseStartAddress, eraseBytesLength)

            if(memType == 'ram'):
                assert status == bootloader.status.kStatus_FlashAddressError
            elif(memType == 'flash'):
                assert status == bootloader.status.kStatus_FlashAlignmentError

def erase_out_of_memory_range(bl, memType, memIndex, usingSbComamnd=False):
    memStartAddress, memEndAddress = common_util.get_memory_start_end_address(bl, memType, memIndex)
    eraseStartAddress = memEndAddress + 1
    eraseBytesLength = 0x400
    eraseEndAddress = eraseStartAddress + eraseBytesLength
    if(usingSbComamnd == True):
        bdContent = SBFile.bd_content_init(bl, needSign=False)
        bdContent = SBFile.bd_content_update_for_flash_erase_region(bdContent, eraseStartAddress, eraseEndAddress)
        bdFile = SBFile.bd_content_finish(bl, bdContent)
        sbFile = SBFile.generate_sb_file(bl, bdFile, needEncrypt=False, encryptionKey=None, needSign=False, s=[], S=[], R=[])
        status, results = bl.receive_sb_file(sbFile)
        assert status == bootloader.status.kStatusMemoryRangeInvalid
    else:
        status, results = bl.flash_erase_region(eraseStartAddress, eraseBytesLength)
        assert status == bootloader.status.kStatusMemoryRangeInvalid


# EOF