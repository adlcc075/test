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

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '../..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from PyUnitTests import bltest_config
from common import jlink_debugger
from fsl.bootloader import encryptiontypes
from fsl.bootloader import bootsources

class Target:
    def __init__(self, **kwargs):
        self.targetName     = 'M' + bltest_config.targetName.upper()
        self.deviceName     = bltest_config.deviceName
        self.jlinkUsbId     = bltest_config.jlinkUsbId
        self.projectName    = bltest_config.projectName
        self.buildVersion   = bltest_config.buildVersion
        self.compiler       = bltest_config.compiler
        self.imageElfFile   = bltest_config.imageElfFile
        self.releasePackage = bltest_config.releasePackage
        self._debugger      = jlink_debugger.JLink(self.jlinkUsbId, self.deviceName)
        
        self.bootloaderType = bootsources.bootSourcesDict[self.projectName]
        self.BL_APP_VECTOR_TABLE_ADDRESS = self._get_app_running_address()
    
        self.isCRCCheckSupported      = self._get_dict_default(kwargs, 'isCRCCheckSupported', False)
        self.isQspiSupported          = self._get_dict_default(kwargs, 'isQspiSupported', False)
        self.isEncryptionSupported    = self._get_dict_default(kwargs, 'isEncryptionSupported', False)
        self.encryptionModuleType     = self._get_dict_default(kwargs, 'encryptionModuleType', encryptiontypes.kEncryptionType_mmCAU)
        self.mmcauBinFileAppPlatform  = self._get_dict_default(kwargs, 'mmcauBinFileAppPlatform', encryptiontypes.kMmcauBinFileAppPlatformType_cm4)

        self.isDecryptionSupported    = self._get_dict_default(kwargs, 'isDecryptionSupported', False)
        self.programAlignmentSize     = self._get_dict_default(kwargs, 'programAlignmentSize', 4)
        self.eraseAlignmentSize       = self._get_dict_default(kwargs, 'eraseAlignmentSize', 8)
        
        self.hasErasableIFR           = self._get_dict_default(kwargs, 'hasErasableIFR', False)
        self.deviceMemoryAccessable   = self._get_dict_default(kwargs, 'isDeviceMemoryAccessable', False)
        
        self.isHwReliableUpdateSupported  = self._get_dict_default(kwargs, 'isHwReliableUpdateSupported', False)
        
        self.isSwReliableUpdateSupported  = self._get_dict_default(kwargs, 'isSwReliableUpdateSupported', False)
        self.backUpAppStart               = self._get_dict_default(kwargs, 'backUpAppStart', False)
        
        self.isSecureBootSupported    = self._get_dict_default(kwargs, 'isSecureBootSupported', False)
        
        self.isDualCoreBootSupported  = self._get_dict_default(kwargs, 'isDualCoreBootSupported', False)
        self.maxSramIndex             = self._get_dict_default(kwargs, 'maxSramIndex', 1)
        self.maxFlashIndex            = self._get_dict_default(kwargs, 'maxFlashIndex', 1)
        self.isSB2Supported           = self._get_dict_default(kwargs, 'isSB2Supported', False)
#         self.kSIM_AddressFor8BitModuleClockGate    = 0x40048034
#         self.kClockGatePosFor8BitModule            = 10                  # UART0
#         self.kAccessAddressFor8BitModuleStart      = 0x4006A002  # UART0_BDH
#         self.kAccessLengthFor8BitModule            = 1
# 
#         self.kSIM_AddressFor16BitModuleClockGate   = 0x4004803c # SIM_SCGC6
#         self.kClockGatePosFor16BitModule           = 1 # DMA
#         self.kAccessAddressFor16BitModuleStart     = 0x40009014
#         self.kAccessLengthFor16BitModule           = 2
# 
#         self.kSIM_AddressFor32BitModuleClockGate   = 0x4004803c # SIM_SCGC6
#         self.kClockGatePosFor32BitModule           = 1    #DMA
#         self.kAccessAddressFor32BitModuleStart     = 0x40009028 # DMA_TCD1_NBYTES_MLNO
#         self.kAccessLengthFor32BitModule           = 8
        
        

    ## @brief Get app location address
    def _get_app_running_address(self):
        if self.bootloaderType == bootsources.kBootFlash_ExecuteFlash:
            if self.targetName == 'MKL25Z4':
                return 0x8000
            elif self.targetName == 'MKV58F22':
                return 0x1000A000
            else:
                return 0xA000
        else:
            return 0x0

    # @breif Get the value of the key from the given dictionary.
    #        If the key is not existing, then return the default.
    def _get_dict_default(self, dictionary, key, default):
        if not dictionary.has_key(key):
            return default
        else:
            return dictionary[key]

    # @brief Reset device by JLink.
    def reset(self):
        self._debugger.reset()

    # @brief Unlock the Kinetis device by JLink.
    def unlock(self):
        self._debugger.unlock()

    # @brief Flash image by JLinkGDBServer. 
    #        Note that the hexFile and binFile come from the same project.
    def flash_image(self, hexFile, binFile):
        self._debugger.flash_image(hexFile, binFile)

    # @brief Erase the whole internal flash by JLink.
    def erase(self):
        self._debugger.erase()

    # @brief Dump the binary memory from the specific device by JLink GDB server.
    # Write contents of memory to a raw binary file
    def dump_memory(self, address, bytes):
        return self._debugger.dump_memory(address, bytes)

    # @brief Restore the binary file to the device memory.
    # Restore the contents of the binary file to target memory.
    def restore_memory(self, memoryAddress, binFile):
        self._debugger.restore_memory(memoryAddress, binFile)
