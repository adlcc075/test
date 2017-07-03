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

def read_reserved_memory_region(bl, memType):
    (reservedMemStartAddress, reservedMemEndAddress, reservedMemSize) = common_util.get_reserved_memory_region(bl, memType)
    if(reservedMemSize == 0):  # ROM has no reserved flash region
        print('No reserved region for %s memory.' %(memType.upper()))
        return
    else:
        print("\nRead data from reserved memory region:"),  
        readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(reservedMemStartAddress, reservedMemSize, readDataFile)
        assert status == bootloader.status.kStatus_Success

def read_unaligned_memory_address(bl, memType, memIndex):
    (memoryStartAddress, memoryEndAddress) = common_util.get_memory_start_end_address(bl, memType, memIndex)    
    readBytesLength = bl.target.eraseAlignmentSize
    for i in range(1, bl.target.eraseAlignmentSize):
        unalignedAddress = memoryStartAddress + i
        print("\nRead data from unaligned memory address:"),  
        readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(unalignedAddress, readBytesLength, readDataFile)
        assert status == bootloader.status.kStatus_Success
            
def read_unaligned_bytes_length(bl, memType, memIndex):
    (memoryStartAddress, memoryEndAddress) = common_util.get_memory_start_end_address(bl, memType, memIndex)    
    readAddress = memoryStartAddress
    for i in range(1, bl.target.eraseAlignmentSize):
        unalignedByteLength = i
        print("\nRead unaligned byte length of data from memory start address:"),  
        readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(readAddress, unalignedByteLength, readDataFile)
        assert status == bootloader.status.kStatus_Success   

def read_out_of_memory_range(bl, memType, memIndex):
     (memoryStartAddress, memoryEndAddress) = common_util.get_memory_start_end_address(bl, memType, memIndex)
     print("\nRead data out of memory range:"),  
     readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
     status, results = bl.read_memory(memoryEndAddress, 0x400, readDataFile)
     assert status == bootloader.status.kStatusMemoryRangeInvalid
    
    