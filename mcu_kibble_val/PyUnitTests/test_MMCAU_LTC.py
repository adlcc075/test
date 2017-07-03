#! /usr/bin/env python

# Copyright (c) 2015 Freescale Semiconductor, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code mst retain the above copyright notice, this list
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
import time

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from fsl.bootloader import encryptiontypes
from common import common_util
from common import user_config_area
from common import sb_command

#SB command dictionary
sbCmdDict = sb_command.sbCmdDict

# A string for simple sb file
kStringForSimpleSbFile = common_util.kStringForSimpleSbFile
# A function for function sb file
kFormatStringForFunctionSbFile = common_util.kFormatStringForFunctionSbFile

kFlashKeyIFRIndex = [0x30, 0x31, 0x32, 0x33]
BL_FEATURE_ENCRYPTION_KEY_ADDRESS   =   0xb000

MMCAU_FUNCTION_INFO_TAG             =   'kcau'
MMCAU_FUNCTION_INFO_SIZE            =   20
MMCAU_OFFSET_cm0p_aes_init_start    =   0x14
MMCAU_OFFSET_cm0p_aes_encrypt_start =   0xB8
MMCAU_OFFSET_cm0p_aes_decrypt_start =   0x200

MMCAU_OFFSET_cm4_aes_init_start     =   0x14
MMCAU_OFFSET_cm4_aes_encrypt_start  =   0xEA
MMCAU_OFFSET_cm4_aes_decrypt_start  =   0x29A


## @brief Convert hex to chr.
def hex_to_chr(hex):
    chrResult = ''
    chrResult += chr((hex & 0x000000ff) >> 0)
    chrResult += chr((hex & 0x0000ff00) >> 8)
    chrResult += chr((hex & 0x00ff0000) >> 16)
    chrResult += chr((hex & 0xff000000) >> 24)
    return chrResult

## @breif
def get_MMCAU_function_ready(bl, mmcauDataResident):
    # Get mmcau function info
    if bl.target.mmcauBinFileAppPlatform == common_util.kMmcauBinFileAppPlatformType_cm4:
        mmcauFuncFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', 'mmcau_function_cm4.bin'))
        mmcauAddr_aes_init_start        =   MMCAU_OFFSET_cm4_aes_init_start
        mmcauAddr_aes_encrypt_start     =   MMCAU_OFFSET_cm4_aes_encrypt_start
        mmcauAddr_aes_decrypt_start     =   MMCAU_OFFSET_cm4_aes_decrypt_start
    elif bl.target.mmcauBinFileAppPlatform == common_util.kMmcauBinFileAppPlatformType_cm0p:
        mmcauFuncFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', 'mmcau_function_cm0p.bin'))
        mmcauAddr_aes_init_start        = MMCAU_OFFSET_cm0p_aes_init_start
        mmcauAddr_aes_encrypt_start     = MMCAU_OFFSET_cm0p_aes_encrypt_start
        mmcauAddr_aes_decrypt_start     = MMCAU_OFFSET_cm0p_aes_decrypt_start
    else:
        raise ValueError('Invalid mmcauBinFileAppPlatform.')
    # mmcauFuncFileLen = 0x4BC bytes for mmcau_function_cm4.bin
    # mmcauFuncFileLen = 0x380 bytes for mmcau_function_cm0p.bin
    mmcauFuncFileLen = os.path.getsize(mmcauFuncFilePath)

    # Memory address for MMCAU data. The address must be at least 4 bytes aligment because it is a function pointer.
    memStartAddr = common_util.get_memory_start_address(bl, mmcauDataResident)
    memTotalSize = common_util.get_memory_total_size(bl, mmcauDataResident)
    mmcauStart = memStartAddr + memTotalSize - MMCAU_FUNCTION_INFO_SIZE - mmcauFuncFileLen
    # Here set 16 bytes down alignment.
    mmcauStart = mmcauStart - (mmcauStart & 0xF)

    # Create MMCAU info file
    mmcauAddr_aes_init_start += mmcauStart
    mmcauAddr_aes_encrypt_start += mmcauStart
    mmcauAddr_aes_decrypt_start += mmcauStart
    mmcauFunctionInfo = ''
    mmcauFunctionInfo += MMCAU_FUNCTION_INFO_TAG
    mmcauFunctionInfo += hex_to_chr(mmcauFuncFileLen)
    mmcauFunctionInfo += hex_to_chr(mmcauAddr_aes_init_start)
    mmcauFunctionInfo += hex_to_chr(mmcauAddr_aes_encrypt_start)
    mmcauFunctionInfo += hex_to_chr(mmcauAddr_aes_decrypt_start)
    mmcauFuncInfoFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', 'mmcau_function_info.bin'))
    with open(mmcauFuncInfoFilePath, 'wb') as fileObj:
        fileObj.write(mmcauFunctionInfo)
        fileObj.close

    # Updata the BCA data for mmcauConfigPointer and write BCA data to BL_APP_VECTOR_TABLE_ADDRESS
    update_MMCAU_config_pointer_in_BCA(bl, mmcauConfigPointer = mmcauStart)

    # Write MMCAU data to memory. MMCAU data includes two parts,
    print("Place the mmcau function info to the mmcauConfigPointer = 0x%x:" %mmcauStart)
    print("(Note: the info includes the aes_init, aes_encrypt, aes_decrypt function addresses)"),
    status, results = bl.write_memory(mmcauStart, mmcauFuncInfoFilePath)
    assert status == bootloader.status.kStatus_Success
    print("Place the aes_init, aes_encrypt, aes_decrypt function codes to the %s address 0x%x:" %(mmcauDataResident, mmcauStart + MMCAU_FUNCTION_INFO_SIZE)),
    status, results = bl.write_memory(mmcauStart + MMCAU_FUNCTION_INFO_SIZE, mmcauFuncFilePath)
    assert status == bootloader.status.kStatus_Success

    return mmcauStart

## @breif This function is to update the mmcauConfigPointer in the BCA.
def update_MMCAU_config_pointer_in_BCA(bl, mmcauConfigPointer):
    # Generate BCA data and write to the BCA.dat file
    bcaFilePath = user_config_area.create_BCA_data_file(bl, mmcauConfigPointer = mmcauConfigPointer)

    # # Write the BCA configuration data to flash
    print("Update the mmcauConfigPointer in the BCA (mmcauConfigPointer = 0x%x):" %mmcauConfigPointer),
    status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
    assert status == bootloader.status.kStatus_Success

## @breif This function is to program the AES encryption key.
#         For ROM, the key will be programmed to the flash IFR (IFR index: 0x30, 0x31, 0x32, 0x33).
#         For bootloader, the key will be programmed to the specified flash address which is 0xb000 in KBL 2.0.
def program_encryption_key(bl, key):
    if bl.target.hasErasableIFR:
        key = sb_command.convert_32bit_key(key)
        print("Program the encryption key to the IFR index 0x30, 0x31, 0x32, 0x33:"),
        for i in range(0, len(key)):
            status, results = bl.flash_program_once(kFlashKeyIFRIndex[i], 4, key[i])
            assert status == bootloader.status.kStatus_Success
    else:
        keyBinFile = os.path.join(bl.vectorsDir, 'key.bin')
        fileObj = file(keyBinFile, 'wb+')
        fileObj.write(key.decode('hex'))
        fileObj.close()
        print("Program the encryption key to the specific address 0x%x:" %BL_FEATURE_ENCRYPTION_KEY_ADDRESS),
        status, results = bl.write_memory(BL_FEATURE_ENCRYPTION_KEY_ADDRESS, keyBinFile)
        assert status == bootloader.status.kStatus_Success
        os.remove(keyBinFile)
    print("-"*60 + " Finished programming the encryption key.")

## @breif Verify the data in the memory address
def verify_data(bl, address, data):
    readDataFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'read_data_from_memory.bin'))
    status, results = bl.read_memory(address, len(data), readDataFilePath)
    assert status == bootloader.status.kStatus_Success

    writeBytes = open(readDataFilePath, 'rb').read()
    assert writeBytes == data

## @breif Initialize the sbCmdDict which will be used to generate sb file in sb_command.py. 
def init_sbCmdDict(bl, encryptionType, dataType, memType):
    startAddress, endAddress, length = common_util.get_available_memory_region(bl, memType)
    sbCmdDict['writeMemory'].cumulativeWrite = False
    sbCmdDict['writeMemory'].dataType = dataType
    if dataType == 'string':
        sbCmdDict['writeMemory'].data = kStringForSimpleSbFile
        sbCmdDict['writeMemory'].length = len(kStringForSimpleSbFile)
        sbCmdDict['writeMemory'].startAddress = startAddress
        sbCmdDict['writeMemory'].endAddress = startAddress + len(kStringForSimpleSbFile)
    elif dataType == 'function':
        sbCmdDict['writeMemory'].data = kFormatStringForFunctionSbFile
        sbCmdDict['writeMemory'].length = len(kFormatStringForFunctionSbFile)
        sbCmdDict['writeMemory'].startAddress = startAddress
        sbCmdDict['writeMemory'].endAddress = startAddress + len(kFormatStringForFunctionSbFile)
    elif dataType == 'app_bin':
        (elfFile, hexFile, binFile) = common_util.get_led_demo_path(bl)
        sbCmdDict['writeMemory'].data = binFile
        sbCmdDict['writeMemory'].length = os.path.getsize(binFile)
        sbCmdDict['writeMemory'].startAddress = bl.target.BL_APP_VECTOR_TABLE_ADDRESS
        sbCmdDict['writeMemory'].endAddress = bl.target.BL_APP_VECTOR_TABLE_ADDRESS + os.path.getsize(binFile)


class TestDecryptWithFlashUnsecure:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)  
        def teardown():
            if bl.target.hasErasableIFR:
                bl.target.unlock()
            else:
                pass
        request.addfinalizer(teardown)
    @pytest.mark.parametrize(('encryptionType', 'dataType', 'mmcauDataResident'),
                            [
#-------------------------------------------------------------
# If chip is unsecure, target can receive unencypted sb file.
#-------------------------------------------------------------
                            # Receive unencrypted sb file that contains a simple string
                            ('unencrypted', 'string', 'flash'),
                            # Receive unencrypted sb file that contains a function
                            ('unencrypted', 'function', 'flash'),
                            # Receive unencrypted sb file that contains an application
                            ('unencrypted', 'app_bin', 'flash'),
#----------------------------------------------------------------------
# If chip is unsecure, target can receive zero key encypted sb file.
#----------------------------------------------------------------------
                            # Encrypt a simple string with a zero key, mmcua data in flash
                            ('zeroKeyEncrypted', 'string', 'flash'),
                            # Encrypt a simple string with a zero key, mmcua data in ram
                            ('zeroKeyEncrypted', 'string', 'ram'),
                            # Encrypt a function with a zero key, mmcua data in flash
                            ('zeroKeyEncrypted', 'function', 'flash'),
                            # Encrypt a function with a zero key, mmcua data in ram
                            ('zeroKeyEncrypted', 'function', 'ram'),
                            # Encrypt an application with a zero key, mmcua data in ram
                            ('zeroKeyEncrypted', 'app_bin', 'ram'),
#-----------------------------------------------------------------------
# If chip is unsecure, target can receive non-zero key encypted sb file.
#-----------------------------------------------------------------------
                            # Encrypt a simple string with a non-zero key, mmcua data in flash
                            ('nonZeroKeyEncrypted', 'string', 'flash'),
                            # Encrypt a simple string with a non-zero key, mmcua data in ram
                            ('nonZeroKeyEncrypted', 'string', 'ram'),
                            # Encrypt a function with a non-zero key, mmcua data in flash
                            ('nonZeroKeyEncrypted', 'function', 'flash'),
                            # Encrypt a function with a non-zero key, mmcua data in ram
                            ('nonZeroKeyEncrypted', 'function', 'ram'),
                            # Encrypt an application with a non-zero key, mmcua data in ram
                            ('nonZeroKeyEncrypted', 'app_bin', 'ram')
                            ])
    def test_receive_sb_file_with_flash_unsecure(self, bl, encryptionType, dataType, mmcauDataResident):
        if encryptionType == 'unencrypted':
            # Initialize the sbCmdDict which will be used to generate sb file in sb_command.py.
            init_sbCmdDict(bl, encryptionType, dataType, mmcauDataResident)
            # Generate sb file
            if dataType == 'app_bin':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, '', 'writeMemory', 'reset')
            elif dataType == 'string' or dataType == 'function':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, '', 'writeMemory')
            # Send the unencrypted sb file to target.
            status, results = bl.receive_sb_file(sbFilePath)
            # Delete the sb file
            os.remove(sbFilePath)
            if dataType == 'string':
                assert status == bootloader.status.kStatus_Success
 
                # Verify the string in the memory address
                verify_data(bl, sbCmdDict['writeMemory'].startAddress, sbCmdDict['writeMemory'].data)
            elif dataType == 'function':
                assert status == bootloader.status.kStatus_Success
 
                # Call the function should pass.
                status, results = bl.call(sbCmdDict['writeMemory'].startAddress + 1, 0)
                assert status == bootloader.status.kStatus_Success
            elif dataType == 'app_bin':
                # After excuting receive-sb-file the led will blink immediately and blhost returns kStatus_AbortDataPhase.
                assert status == bootloader.status.kStatus_AbortDataPhase
                # Let led blink for some time so that we can see the correct phenomenon.
                time.sleep(8)
                # Send command to the target will have no response.
                status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
                assert status != bootloader.status.kStatus_Success
        elif bl.target.isEncryptionSupported == True:
            # 1. Program the encryption key to the specified area
            if encryptionType == 'zeroKeyEncrypted':
                key = '00000000000000000000000000000000'
                status = program_encryption_key(bl, key)
            elif encryptionType == 'nonZeroKeyEncrypted':
                # The encryption key is a string type with length of 32, each character must be 0~F.
                # Generate a random encryption key.
                keyValueFrom = '0123456789ABCDEF'
                key = ''.join(random.sample(keyValueFrom, 16) + random.sample(keyValueFrom, 16))
                status = program_encryption_key(bl, key)
            else:
                raise ValueError('Invalid encryptionType parameter.')
             
 
            # 2. Write BCA data and mmcau data to memory.
            if bl.target.encryptionModuleType == encryptiontypes.kEncryptionType_LTC:
                pass
            elif bl.target.encryptionModuleType == encryptiontypes.kEncryptionType_mmCAU:
                # In the ROM code such as K80, L5K, though it supports the hardware security 
                # encryption/decryption operations via CAU coprocessor which is connected to PPB, 
                # it uses the software to implement the security encryption/decryption operations.
                # So we must place the mmCAU security functions to the mmcauConfigPointer address
                # whose value will be given in the Bootloader Configuration Area (BCA).                # 
                #
                # While for the flash-resident bootloader, it uses the hardware implementation if 
                # the target supports it, so it's no need to use the mmCAU security functions and 
                # operate the Bootloader Configuration Area (BCA).
                if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
                    mmcauDataAddress = get_MMCAU_function_ready(bl, mmcauDataResident)
                elif bl.target.bootloaderType == bootsources.kBootFlash_ExecuteFlash:
                    pass
                 
            else:
                raise ValueError('Invalid encryptionModuleType.')
 
            # 3. Reset the target so that the BCA data can be used.
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            # Wait for some time to make sure the target is successfully reset.
            time.sleep(2)
 
            # 4. Send sb file to target
            # 4.1 Initialize the sbCmdDict which will be used to generate sb file in sb_command.py.
            init_sbCmdDict(bl, encryptionType, dataType, mmcauDataResident)
            # 4.2 Generate sb file
            if dataType == 'app_bin':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, key, 'writeMemory', 'reset')
            elif dataType == 'string' or dataType == 'function':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, key, 'writeMemory')
            # 4.3 Send the encrypted sb file to target.
            status, results = bl.receive_sb_file(sbFilePath)
            # 4.4 Delete the sb file
            os.remove(sbFilePath)
 
            if dataType == 'string':
                assert status == bootloader.status.kStatus_Success
            elif dataType == 'function':
                assert status == bootloader.status.kStatus_Success
 
                status, results = bl.call(sbCmdDict['writeMemory'].startAddress + 1, 0)
                assert status == bootloader.status.kStatus_Success
            elif dataType == 'app_bin':
                # After excuting receive-sb-file the led will blink immediately and blhost returns kStatus_AbortDataPhase.
                assert status == bootloader.status.kStatus_AbortDataPhase
                # Let led blink for some time so that we can see the correct phenomenon.
                time.sleep(8)
                # Send command to the target will have no response.
                status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
                assert status != bootloader.status.kStatus_Success
        else:
            pytest.skip("This case should be skipped, because current test platform doesn't support security related feature.")

        

# @breif Test sb file encryption and mmacu/LTC decryption when flash is secure
class TestDecryptWithFlashSecure:
     @pytest.fixture(autouse=True)
     def setup(self, request, bl):
         if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
            bl.target.unlock()
            common_util.reset_without_check(bl)
            # erase all the flash to let flash 0x40c be 1s
            common_util.erase_all_available_flash(bl)
         else:
            pytest.skip("This case only supports on ROM-resident bootloader.")
         def teardown():
             # We should use JLink to unlock the device so that JLink can connect with the device and reset the device.
             bl.target.unlock()
         request.addfinalizer(teardown)

     @pytest.mark.parametrize(('encryptionType', 'dataType', 'mmcauDataResident'),
                             [
 #-------------------------------------------------------------
 # If chip is secure, target cannot receive unencypted sb file.
 #-------------------------------------------------------------
                           # Receive unencrypted sb file that contains a simple string
                           ('unencrypted', 'string', 'flash'),
                           # Receive unencrypted sb file that contains a function
                           ('unencrypted', 'function', 'flash'),
                           # Receive unencrypted sb file that contains an application
                           ('unencrypted', 'app_bin', 'flash'),
#----------------------------------------------------------------------
# If chip is secure, target can receive zero key encypted sb file.
#----------------------------------------------------------------------
                            # Encrypt a simple string with a zero key, mmcua data in flash
                            ('zeroKeyEncrypted', 'string', 'flash'),
                            # Encrypt a simple string with a zero key, mmcua data in ram
                            ('zeroKeyEncrypted', 'string', 'ram'),
                            # Encrypt a function with a zero key, mmcua data in flash
                            ('zeroKeyEncrypted', 'function', 'flash'),
                            # Encrypt a function with a zero key, mmcua data in ram
                            ('zeroKeyEncrypted', 'function', 'ram'),
                            # Encrypt an application with a zero key, mmcua data in ram
                            ('zeroKeyEncrypted', 'app_bin', 'ram'),
 #-----------------------------------------------------------------------
 # If chip is secure, target can receive non-zero key encypted sb file.
 #-----------------------------------------------------------------------
                             # Encrypt a simple string with a non-zero key, mmcua data in flash
                            ('nonZeroKeyEncrypted', 'string', 'flash'),
                            # Encrypt a simple string with a non-zero key, mmcua data in ram
                            ('nonZeroKeyEncrypted', 'string', 'ram'),
                            # Encrypt a function with a non-zero key, mmcua data in flash
                            ('nonZeroKeyEncrypted', 'function', 'flash'),
                            # Encrypt a function with a non-zero key, mmcua data in ram
                            ('nonZeroKeyEncrypted', 'function', 'ram'),
                            # Encrypt an application with a non-zero key, mmcua data in ram
                            ('nonZeroKeyEncrypted', 'app_bin', 'ram')
                             ])
     def test_receive_sb_file_with_flash_secure(self, bl, encryptionType, dataType, mmcauDataResident):
        if encryptionType == 'unencrypted':
            # Reset the target after flash-erase-all in the setup function so that flash is in secure state.
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            # Wait for some time to make sure the target is successfully reset.
            time.sleep(2)
            # Check the state of the flash, should be in secure
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert results[0] == 1
            
            # Initialize the sbCmdDict which will be used to generate sb file in sb_command.py.
            init_sbCmdDict(bl, encryptionType, dataType, mmcauDataResident)
            # Generate sb file
            if dataType == 'app_bin':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, '', 'writeMemory', 'reset')
            elif dataType == 'string' or dataType == 'function':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, '', 'writeMemory')
            # Send the unencrypted sb file to target.
            status, results = bl.receive_sb_file(sbFilePath)
            # Target cannot receive the unencrypted sb file when flash is secure
            assert status == bootloader.status.kStatusRomLdrSecureOnly
            # Delete the sb file
            os.remove(sbFilePath)
            
        elif bl.target.isEncryptionSupported == True:
            # 1. Program the encryption key to the specified area
            if encryptionType == 'zeroKeyEncrypted':
                key = '00000000000000000000000000000000'
                program_encryption_key(bl, key)
            elif encryptionType == 'nonZeroKeyEncrypted':
                # The encryption key is a string type with length of 32, each character must be 0~F.
                # Generate a random encryption key.
                keyValueFrom = '0123456789ABCDEF'
                key = ''.join(random.sample(keyValueFrom, 16) + random.sample(keyValueFrom, 16))
                program_encryption_key(bl, key)
            else:
                raise ValueError('Invalid encryptionType parameter.')
            

            # 2. Write BCA data and mmcau data to memory.
            if bl.target.encryptionModuleType == encryptiontypes.kEncryptionType_LTC:
                pass
            elif bl.target.encryptionModuleType == encryptiontypes.kEncryptionType_mmCAU:
                # In the ROM code such as K80, L5K, though it supports the hardware security 
                # encryption/decryption operations via CAU coprocessor which is connected to PPB, 
                # it uses the software to implement the security encryption/decryption operations.
                # So we must place the mmCAU security functions to the mmcauConfigPointer address
                # whose value will be given in the Bootloader Configuration Area (BCA).                # 
                #
                # While for the flash-resident bootloader, it uses the hardware implementation if 
                # the target supports it, so it's no need to use the mmCAU security functions and 
                # operate the Bootloader Configuration Area (BCA).
                if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
                    mmcauDataAddress = get_MMCAU_function_ready(bl, mmcauDataResident)
                elif bl.target.bootloaderType == bootsources.kBootFlash_ExecuteFlash:
                    pass
                
            else:
                raise ValueError('Invalid encryptionModuleType.')

            # 3. Reset the target so that the BCA data can be used, and flash is in secure state.
            #    We can use flash-erase-all and reset to set flash in secure state for ROM.
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            # Wait for some time to make sure the target is successfully reset.
            time.sleep(2)

            # 4. Check the state of the flash, should be in secure
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert results[0] == 1

            # 5. Send sb file to target.
            # 5.1 Initialize the sbCmdDict which will be used to generate sb file in sb_command.py.
            init_sbCmdDict(bl, encryptionType, dataType, mmcauDataResident)
            # 5.2 Generate sb file
            if dataType == 'app_bin':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, key, 'writeMemory', 'reset')
            elif dataType == 'string' or dataType == 'function':
                sbFilePath = sb_command.generate_sb_file(bl, encryptionType, key, 'writeMemory')
            # 5.3 Send the encrypted sb file to target.
            status, results = bl.receive_sb_file(sbFilePath)
            # 5.4 Delete the sb file
            os.remove(sbFilePath)

            if dataType == 'string':
                assert status == bootloader.status.kStatus_Success
            elif dataType == 'function':
                assert status == bootloader.status.kStatus_Success
                # Call the function will fail because flash is secure.
                status, results = bl.call(sbCmdDict['writeMemory'].startAddress + 1, 0)
                assert status == bootloader.status.kStatus_SecurityViolation
            elif dataType == 'app_bin':
                # After excuting receive-sb-file the led will blink immediately and blhost returns kStatus_AbortDataPhase.
                assert status == bootloader.status.kStatus_AbortDataPhase
                # Let led blink for some time so that we can see the correct phenomenon.
                time.sleep(8)
                # Send command to the target will have no response.
                status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
                assert status != bootloader.status.kStatus_Success

        else:
            pytest.skip("This case should be skipped, because current test platform doesn't support security related feature.")

        