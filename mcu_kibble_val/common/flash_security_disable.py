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
import time

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util
from common import flash_config_area

def flash_security_disable(bl, backdoorKey, bootCore):
    if(bl.target.targetName.upper() == 'MK3S'):
        flash_security_disable_for_new_targets(bl, backdoorKey, bootCore)
    else:
        flash_security_disable_for_legacy_targets(bl, backdoorKey)

def flash_security_disable_for_legacy_targets(bl, backdoorKey):
    # 1. Erase all the flash, including the flash configuration data (0x400 ~ 0x40F)
    status, results = bl.flash_erase_all()
    assert status == bootloader.status.kStatus_Success
    # 2. Generate flash configuration data
    flashConfigDataFile = flash_config_area.generate_flash_config_data(
                          backdoorKeyByte0      = int(backdoorKey[ 0: 2], 16),
                          backdoorKeyByte1      = int(backdoorKey[ 2: 4], 16),
                          backdoorKeyByte2      = int(backdoorKey[ 4: 6], 16),
                          backdoorKeyByte3      = int(backdoorKey[ 6: 8], 16),
                          backdoorKeyByte4      = int(backdoorKey[ 8:10], 16),
                          backdoorKeyByte5      = int(backdoorKey[10:12], 16),
                          backdoorKeyByte6      = int(backdoorKey[12:14], 16),
                          backdoorKeyByte7      = int(backdoorKey[14:16], 16),
                          # here should enable backdoor key access
                          flashSecurityRegister = 0xbf)
    # 3. Update the flash configuration area (0x400 ~ 0x40F)
    print("\nProgram 8 bytes Backdoor Comparison Key 0x%s to flash configuration area 0x400~0x407,"%backdoorKey)
    print("and program 0xBF to flash address 0x408 to enable backdoor key access:"),
    status, results = bl.write_memory(0x400, flashConfigDataFile)
    assert status == bootloader.status.kStatus_Success
    # 4. Reset target
    print('\nReset target to enable flash security:'),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    time.sleep(2)
    # 5. Check if flash is in secure state
    print("\nCheck if flash is in secure state:"),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
    assert status == bootloader.status.kStatus_Success
    assert results[0] == 1
    # 6. Flash cannot be accessed when it is in secure state
    print("\nFlash cannot be accessed when it is in secure state:"),  
    readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
    status, results = bl.read_memory(0x0, 0x400, readDataFile)
    # 7. Use backdoor key to disable flash security.
    print("\nUse backdoor key to disable flash security:"),
    status, results = bl.flash_security_disable(backdoorKey)
    if backdoorKey == 'FFFFFFFFFFFFFFFF' or backdoorKey == '0000000000000000':
        print('NOTE: If the entire 8-byte key is all zeros or all ones,')
        print('      the Verify Backdoor Access Key command fails with an access error.')
        assert status == bootloader.status.kStatus_FlashAccessError
    else:    
        assert status == bootloader.status.kStatus_Success
        # 8. Check if flash is in unsecure state
        print("\nAfter using the backdoor key to disable the flash security, flash should be in unsecure state:"),
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
        assert status == bootloader.status.kStatus_Success
        assert results[0] == 0
        # 9. Flash can be accessed when it is in unsecure state
        print("\nFlash can be accessed now because the flash security is disabled by the backdoor key:"),  
        readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(0x0, 0x400, readDataFile) 
        assert status == bootloader.status.kStatus_Success
        # 10. Flash will be in secure state after reset
        print('\nReset target will enable flash security:'),
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        print("\nCheck if flash is in secure state:"),
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
        assert status == bootloader.status.kStatus_Success
        assert results[0] == 1
    print("\nReleasing the flash security state:"),
    status, results = bl.flash_erase_all_unsecure()
    assert status == bootloader.status.kStatus_Success
    
def flash_security_disable_for_new_targets(bl, backdoorKey, bootCore):
    IFR_coreSwitch = 0x84
    IFR_backDoorKeyEnable = 0x83
    IFR_backDoorComparisonKey = 0xB0
    
    # Erase the whole flash regions
    status, results = bl.flash_erase_all()
    assert status == bootloader.status.kStatus_Success
    # Switch the boot core
    if(bootCore == 'core0'):    # M4 core for K3S
        print('\nSwitch the boot core to core0:'),
    elif(bootCore == 'core1'):  # M0+ core for K3S
        print('\nSwitch the boot core to core1:'),
        status, results = bl.flash_program_once(IFR_coreSwitch, 4, 'ffffffbf', 'lsb')
        assert status == bootloader.status.kStatus_Success
    # Enable the backdoor key access
    print('\nEnable the backdoor key access:'),
    status, results = bl.flash_program_once(IFR_backDoorKeyEnable, 4, 'ffffff00', 'lsb')
    assert status == bootloader.status.kStatus_Success
    # Set the backdoor key to the corresponding IFR
    print('\nSet the backdoor key to the corresponding IFR:'),
    status, results = bl.flash_program_once(IFR_backDoorComparisonKey, 8, backdoorKey, 'msb')
    assert status == bootloader.status.kStatus_Success
    # Reset tne target, so that the IFR values will be loaded and flash will be secure state.
    print('\nReset target to 1) switch boot core, 2) enable flash security.'),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    time.sleep(2)
    # 5. Check if flash is in secure state
    print("\nCheck if flash is in secure state:"),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
    assert status == bootloader.status.kStatus_Success
    assert results[0] == 1
    # 6. Flash cannot be accessed when it is in secure state
    print("\nFlash cannot be accessed when it is in secure state:"),  
    readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
    status, results = bl.read_memory(0x0, 0x400, readDataFile)
    assert status == bootloader.status.kStatus_SecurityViolation
    # 7. Use backdoor key to disable flash security.
    print("\nUse backdoor key to disable flash security:"),
    status, results = bl.flash_security_disable(backdoorKey)
    if backdoorKey == 'FFFFFFFFFFFFFFFF' or backdoorKey == '0000000000000000':
        print('NOTE: If the entire 8-byte key is all zeros or all ones,')
        print('      the Verify Backdoor Access Key command fails with an access error.')
        assert status == bootloader.status.kStatus_FlashAccessError
    else:    
        assert status == bootloader.status.kStatus_Success
        # 8. Check if flash is in unsecure state
        print("\nAfter using the backdoor key to disable the flash security, flash should be in unsecure state:"),
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
        assert status == bootloader.status.kStatus_Success
        assert results[0] == 0
        # 9. Flash can be accessed when it is in unsecure state
        print("\nFlash can be accessed now because the flash security is disabled by the backdoor key:"),  
        readDataFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(0x0, 0x400, readDataFile)
        # 10. Flash will be in secure state after reset
        print('\nReset target will enable flash security:'),
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        print("\nCheck if flash is in secure state:"),
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
        assert status == bootloader.status.kStatus_Success
        assert results[0] == 1
    print("\nReleasing the flash security state:"),
    status, results = bl.flash_erase_all_unsecure()
    assert status == bootloader.status.kStatus_Success

