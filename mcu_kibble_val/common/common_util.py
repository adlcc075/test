# coding:utf-8
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


import os
import sys
import elf
import time
import random
import string
import glob
import shutil
import pytest

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath)  # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from common import compiler

# Constants
kInvalidParameter = 0
kZeroSizeBlock = 1
kValidMemoryRange = 2
kInvalidMemoryRange = 3

# A string for simple sb file
kStringForSimpleSbFile = 'Hello sb loader world! Just for testing'

# A function for function sb file
kFormatStringForFunctionSbFile = "{{05 20 00 bf 40 1e fc d1 70 47}}"

# # @brief Set the sleep time after JLink reset.
SLEEP_SECONDS_AFTER_RESET = 4

# # @breif Delete all the specified types of files
# @param: filePath:          The location of the specified extension files.
#         fileNameExtension: A list that shows the specified file name extensions. 
#                            e.g. ['*.bin', '*.txt', '*.init', '*.jlink']
def batch_remove(filePath, fileNameExtension):
    for extension in fileNameExtension: 
        for tempFile in glob.glob(os.path.join(filePath, extension)):
            try:
                os.remove(tempFile)
            except IOError as e:
                print e
            except WindowsError as e:
                print e

# # @breif This function is to generate random data file with the located start address and specified size.
def generate_random_data_file(bl, startAddress, byteCount):
    randomBytes = bytearray(byteCount)
    for num in range(0, byteCount):
        randomBytes[num] = random.randint(0, 0xFFFF) & 0xFF

    # Fill the flash config area with {{FF FF FF FF FF FF FF FF FF FF FF FF FE FF FF FF}}
    endAddress = startAddress + byteCount - 1
    if startAddress >= 0x410 or endAddress < 0x400:  # pass if random data never covers the flash config area
        pass
    else:
        for index, flashConfigAddress in enumerate(range(0x400, 0x410)):
            addressOffset = endAddress - flashConfigAddress
            if addressOffset >= 0:
                if index == 0xC:
                    randomBytes[flashConfigAddress - startAddress] = 0xFE  # 0x40C
                else:
                    randomBytes[flashConfigAddress - startAddress] = 0xFF
            else:
                pass

    randomFile = os.path.join(bl.vectorsDir, 'generatedBinary%s.bin'%''.join(random.sample(string.digits, 5)))
    with open(randomFile, 'wb+') as file:
        # Clean the file.
        file.truncate()
        file.write(randomBytes)
        file.close()

    return randomFile
# # @brief This function is to generate data file according to the filled pattern
def generate_file_according_to_filled_pattern(bl, pattern, format, length):
    filledBytes = bytearray(length)
    if format == 'byte':
        pattern = pattern & 0xFF
        for i in range(0, length, 4):
            filledBytes[i] = pattern
            filledBytes[i + 1] = pattern
            filledBytes[i + 2] = pattern
            filledBytes[i + 3] = pattern
    elif format == 'short':
        pattern = pattern & 0xFFFF
        for i in range(0, length, 4):
            filledBytes[i] = pattern & 0xFF
            filledBytes[i + 1] = (pattern >> 8) & 0xFF
            filledBytes[i + 2] = 0x00
            filledBytes[i + 3] = 0x00
    elif format == 'word':
        pattern = pattern & 0xFFFFFFFF
        for i in range(0, length, 4):
            filledBytes[i] = pattern & 0xFF
            filledBytes[i + 1] = (pattern >> 8) & 0xFF
            filledBytes[i + 2] = (pattern >> 16) & 0xFF
            filledBytes[i + 3] = (pattern >> 24) & 0xFF

    filledDataFile = os.path.join(bl.vectorsDir, 'write_data_to_memory.bin')
    with open(filledDataFile, 'wb+') as file:
        # Clean the file.
        file.truncate()
        file.write(filledBytes)
        file.close()

    return filledDataFile

# # @breif This function is to get SP and PC values from an app demo bin file.
def get_sp_pc_from_bin_file(appBinFilePath):
    with open(appBinFilePath, 'r') as file:
        # The first four bytes are stack pointer
        file.seek(0)
        spStr = file.read(4)
        # The second four bytes are PC pointer
        file.seek(4)
        pcStr = file.read(4)
        file.close()

    spInt = ord(spStr[0]) | (ord(spStr[1]) << 8) | (ord(spStr[2]) << 16) | (ord(spStr[3]) << 24)
    pcInt = ord(pcStr[0]) | (ord(pcStr[1]) << 8) | (ord(pcStr[2]) << 16) | (ord(pcStr[3]) << 24)
    return spInt, pcInt

# # @breif This function is to get the led demo path.
# 1. The led demo follows the below naming rule:
#    boardType + bl.target.targetName + 'led_demo_' + bl.target.compiler
#         e.g. tower_kl28z7_led_demo_iar, freedom_kl28z7_led_demo_mdk ...
#  or
#    boardType + bl.target.targetName + 'led_demo_qspi_' + bl.target.compiler
#         e.g. tower_kl28z7_led_demo_qspi_iar, freedom_kl28z7_led_demo_qspi_kds ...
# 2. We assume that the led demo is in the "..\mcu_kibble_val\targets\$targetName" directory, and return it directly.
# 3. If not exist in default, then build the led project in the kibble release package to generate the image file.

def get_led_demo_path(bl, runInQspiFlash = False):
    # Get the board information from bl.target.projectName
    if 'fpga' in bl.target.projectName:
        boardType = 'fpga'
    elif 'freedom' in bl.target.projectName:
        boardType = 'freedom'
    elif 'tower' in bl.target.projectName:
        boardType = 'tower'
    else:
        boardType = 'tower' # run the flashloader and flashloader_loader on the tower board

    # Get the led demo name
    if runInQspiFlash:
        pass
        # to-do
#         ledDemoName = '%s_%s_led_demo_qspi_%s' %(boardType, bl.target.targetName.lower(), bl.target.compiler)
#         ledDemoProjectFolder = os.path.join(bl.bl.target.releasePackage, 'apps/demo_qspi/led_demo/targets')
    else:
#         ledDemoName = '%s_%s_led_demo_%s_0x%x' %(boardType, bl.target.targetName.lower(), bl.target.compiler, bl.target.BL_APP_VECTOR_TABLE_ADDRESS)
        ledDemoName = '%s_%s_led_demo_%s_0x%x' %(boardType, bl.target.targetName.lower(), 'iar', bl.target.BL_APP_VECTOR_TABLE_ADDRESS)
        ledDemoProjectFolder = os.path.join(bl.target.releasePackage, 'apps/led_demo', "%s"%bl.target.targetName)


    # This function returns 3 image formats (elf, hex, bin) for the same demo.
    # If the images exist, then return the elf, hex, bin image file directly.
    ledDemoLocation = os.path.join('%s/../../../targets/%s' %(bl.vectorsDir, bl.target.targetName.lower().split('m',1)[1]))
    elfFile = os.path.abspath(os.path.join(ledDemoLocation, "%s.elf" %(ledDemoName)))
    hexFile = os.path.abspath(os.path.join(ledDemoLocation, "%s.hex" %(ledDemoName)))
    binFile = os.path.abspath(os.path.join(ledDemoLocation, "%s.bin" %(ledDemoName)))
    if os.path.exists(elfFile) and os.path.exists(hexFile) and os.path.exists(binFile):
        return (elfFile, hexFile, binFile)
    # If the images do not exist, then build the led project in the kibble release package to generate the image file. 
    else:
        if bl.target.BL_APP_VECTOR_TABLE_ADDRESS == 0x0:
            projectName = 'led_demo_%s_0000' %(boardType)
        else:
            projectName = 'led_demo_%s_%s' %(boardType, hex(bl.target.BL_APP_VECTOR_TABLE_ADDRESS).split('0x')[1])
        
        # Currently only use IAR to build the led demo.
        ewpFilePath = os.path.join(ledDemoProjectFolder, 'iar', '%s' %(projectName), '%s.ewp' %(projectName))
        if os.path.exists(ewpFilePath):
            (elfFile, hexFile, binFile) = compiler.Compiler().iar_build(ewpFilePath, 'debug')
        else:
            print("Cannot build the led demo project as it does not exist:\n%s" %ewpFilePath)
            raise ValueError('Cannot build the led demo project as it does not exist.')
            
#         if bl.target.compiler == 'iar':
#             ewpFilePath = os.path.join(ledDemoProjectFolder, 'iar', '%s' %(projectName), '%s.ewp' %(projectName))
#             if os.path.exists(ewpFilePath):
#                 (elfFile, hexFile, binFile) = compiler.Compiler().iar_build(ewpFilePath, 'debug')
#             else:
#                 print("Cannot build the led demo project as it does not exist:\n%s" %ewpFilePath)
#                 raise ValueError('Cannot build the led demo project as it does not exist.')
#         elif bl.target.compiler == 'kds':
#             kdsProjectPath = os.path.join(ledDemoProjectFolder, 'kds', '%s' %(projectName))
#             if os.path.exists(kdsProjectPath):
#                 (elfFile, hexFile, binFile) = compiler.Compiler().kds_build(kdsProjectPath, 'debug')
#             else:
#                 print("Cannot build the led demo project as it does not exist:\n%s" %kdsProjectPath)
#                 raise ValueError('Cannot build the led demo project as it does not exist.')
#         elif bl.target.compiler == 'mdk':
#             uvprojxFilePath = os.path.join(ledDemoProjectFolder, 'mdk', '%s' %(projectName), '%s.uvprojx' %(projectName))
#             if os.path.exists(uvprojxFilePath):
#                 (elfFile, hexFile, binFile) = compiler.Compiler().mdk_build(uvprojxFilePath, 'debug')
#             else:
#                 print("Cannot build the led demo project as it does not exist:\n%s" %uvprojxFilePath)
#                 raise ValueError('Cannot build the led demo project as it does not exist.')
#         else:
#             print('Do not support %s build currently.' %bl.target.compiler)
#             raise ValueError("Do not support this compiler, please check.")
        # Copy the images to one specified location and rename them.
        for image in (elfFile, hexFile, binFile):
            imageCopy = os.path.join(ledDemoLocation, '%s%s' %(ledDemoName, os.path.splitext(image)[1]))
            shutil.copy(image, imageCopy)
        return (elfFile, hexFile, binFile)

# # @breif Directly reset the target without any check. Should consider different types of bootloader.
def reset_without_check(bl):
    if (bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM) or (bl.target.bootloaderType == bootsources.kBootFlash_ExecuteFlash):
        bl.target.reset()
    elif (bl.target.bootloaderType == bootsources.kBootRAM_ExecuteRAM) or (bl.target.bootloaderType == bootsources.kBootFlash_ExecuteRAM):
        ramResidentBootloaderEntryAddress = get_reset_entry_address(bl)
        bl.target.setPC(ramResidentBootloaderEntryAddress)

    time.sleep(SLEEP_SECONDS_AFTER_RESET)

# # @breif If it can successful communicate between target and blhost, then needn't reset the target, or reset target.
def reset_with_check(bl):
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    if status == bootloader.status.kStatus_Success:
        pass
    else:
        reset_without_check(bl)

# # @breif Set up the test environment for different types of bootloader
# @param: bl
#         bootCore           'core0'      Swtich to M4 core (for K3S).
#                            'core1'      Switch to M0+ core (for K3S). 
#                                         Note this parameter is valid only if the target supports dual core boot.
#         needFlashEraseAll   True        When need to program flash address 0x40C for Kinetis MCU.
#                             False       When operating ram memory.
def setup_test_environment(bl, bootCore, needFlashEraseAll):        
    # For the ROM test, directly unlock the device to clean up the environment
    if (bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM):
        bl.target.unlock()
        time.sleep(2)
        # ==========================================================================================
        if(bl.target.isDualCoreBootSupported):
            if(bootCore == 'core0'):   # core0: M4 (for K3S)
                print('\nSwitch the boot core to core0:'),
            elif(bootCore == 'core1'): # core1: M0 (for K3S)
                print('\nSwitch the boot core to core1:'),
                status, results = bl.flash_program_once(0x84, 4, 'ffffffbf', 'lsb')
                assert status == bootloader.status.kStatus_Success
            # Reset target to switch the boot core
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)
        else:
            pass    # No need to switch boot core because there is single core for this target
        # ==========================================================================================
 
        if(needFlashEraseAll):
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
        else:
            pass
            
    # For the flash-resident bootloader, just reset the target if needed and 
    # erase the availabe flash region.
    elif(bl.target.bootloaderType == bootsources.kBootFlash_ExecuteFlash):
        bl.target.reset()
        if(needFlashEraseAll):
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
        else:
            pass
    # For the flashloader, unlock the device and then download the flashloader image
    elif (bl.target.bootloaderType == bootsources.kBootRAM_ExecuteRAM) or (bl.target.bootloaderType == bootsources.kBootFlash_ExecuteRAM):
        bl.target.unlock()
        # Convert the provided flashloader elf image to hex file and bin file by the corresponding complier tool.
        if(bl.target.compiler == 'iar'):
            (hexFile, binFile) = compiler.Compiler().iar_elf2hex2bin(bl.target.imageElfFile)
        elif(bl.target.compiler == 'keil'):
            (hexFile, binFile) = compiler.Compiler().keil_elf2hex2bin(bl.target.imageElfFile)
        elif(bl.target.compiler == 'kds'):
            (hexFile, binFile) = compiler.Compiler().kds_elf2hex2bin(bl.target.imageElfFile)
        # Flash the flashloader image before the test
        bl.target.flash_image(hexFile, binFile)
        
        if(needFlashEraseAll):
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success
        else:
            pass

#     time.sleep(SLEEP_SECONDS_AFTER_RESET)

# # @brief Get the reset entry address from elf file according to the different types bootloader.
# def get_reset_entry_address(bl):
#     elfData = elf.ELFObject()
#     try:
#         with open(bl.target['elfFileName'], 'rb') as fileObj:
#             elfData.fromFile(fileObj)
#             if elfData.e_type != elf.ELFObject.ET_EXEC:
#                 raise Exception("No executable")
#             resetHandler = elfData.getSymbol("Reset_Handler")
#             vectorRAM = elfData.getSymbol("__VECTOR_RAM")
#     except:
#         print 'cannot process file %s' % elf_file
#         sys.exit(2)
# 
#     if bl.target.bootloaderType == kBootRAM_ExecuteRAM:
#         return resetHandler.st_value - 1
#     elif bl.target.bootloaderType == kBootFlash_ExecuteRAM:
#         return vectorRAM + resetHandler.st_value - 1


# # @brief Erase all the available flash regions and let flash be in the unsecure state.
def erase_all_available_flash_unsecure(bl):
    if bl.target.bootloaderType == bootsources.kBootROM_ExecuteROM:
        status, results = bl.flash_erase_all_unsecure()
        assert status == bootloader.status.kStatus_Success
    elif bl.target.bootloaderType == bootsources.kBootFlash_ExecuteFlash:
        status, results = bl.flash_erase_all()
        assert status == bootloader.status.kStatus_Success
    elif (bl.target.bootloaderType == bootsources.kBootRAM_ExecuteRAM) or (bl.target.bootloaderType == bootsources.kBootFlash_ExecuteRAM):
        status, results = bl.flash_erase_all()
        assert status == bootloader.status.kStatus_Success
        # For these two types bootloader, flash-erase-all command will erase the flash config area,
        # so we must program the flash config area to keep flash in unsecure state.
        status, results = bl.write_memory_with_hex_data(0x400, "{{FFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFF}}")
        assert status == bootloader.status.kStatus_Success

# # @brief Erase all the available flash region including the flash configuration area if it is not reserved.
def erase_all_available_flash(bl):
    status, results = bl.flash_erase_all()
    assert status == bootloader.status.kStatus_Success

# # @brief Set the data to N bytes alignment.
# This function is used in fill-memory and write-memory. The command formats are as follows.
#     -- fill-memory  address  byteCount  pattern  unit
#     -- write-memory address  file
#
# 1) fill-memory and write-memory always need N bytes alignment for the address argument,
#    if not align, blhost always returns kStatus_FlashAlignmentError.
#    The value of N is due to the flash controller. (N = 4 for K22F512, N = 8 for K60_2M, ...)
# 2) The byteCount argument in fill-memory command needs 4 bytes alignment.
#    if not align, blhost will return success, but the actual bytes is the alignment of byteCount.
#    e.g.
#    If byteCount = 1 or 2 or 3, the actual bytes you fill are 4.
#    if byteCount = 25 or 26 or 27, the actual bytes you fill are 28.
def set_data_nbytes_align(data, N):
    alignedData = data
    # If the lowest 2 bits of data are not '00', add binary data '100', i.e. 0x04 to make it 4 bytes align.
    if (data & (N - 1)) != 0:
        alignedData = data - (data & (N - 1)) + N
    return alignedData

# # @brief Validate whether the data is actually filled to the memory when using fill-memory command.
def is_fill_memory_correct(file, dataCount, pattern, unit):
    retValue = True
    # fetchNum is the total number of fetching data from file
    # byteNum is the bytes number when fetching data every time
    if unit == 'byte':
        fetchNum = dataCount
        byteNum = 1
    elif unit == 'short':
        fetchNum = dataCount >> 1
        byteNum = 2
    elif unit == 'word':
        fetchNum = dataCount >> 2
        byteNum = 4
    else:
        raise ValueError('No unit named %s' % unit)
    fileObj = open(file, 'r')

    # Fetch the byteNum data from the file everytime, and compare it with the pattern
    for i in range(0, fetchNum, byteNum):
        # Move the file pointer to the specific offset
        fileObj.seek(i)
        data = fileObj.read(byteNum)
        # Data format convertion (from string type to int type).
        intValue = 0
        for k in range(byteNum - 1, -1, -1):
            intValue = (intValue << 8) | ord(data[k])
        # Compare with the pattern, if not equal then return False.
        if intValue != pattern:
            retValue = False
            break

    fileObj.close()
    return retValue

# # @brief Validate whether the data is actually filled to the memory when using fill-memory command.
def verify_fill_memory(bl, fillAddress, fillLength, fillPattern, patternFormat):
    if(patternFormat == 'byte'):
        verifyData = [fillPattern & 0xFF, fillPattern & 0xFF, fillPattern & 0xFF, fillPattern & 0xFF]
    elif(patternFormat == 'short'):
        verifyData = [fillPattern & 0xFF, (fillPattern >> 8) & 0xFF, fillPattern & 0xFF, (fillPattern >> 8)]
    elif(patternFormat == 'word'):
        verifyData = [fillPattern & 0xFF, (fillPattern >> 8) & 0xFF, (fillPattern >> 16) & 0xFF, (fillPattern >> 24) & 0xFF]
    
    print("\nRead back the data from memory to verify fill-memory:"),  
    readDataFile = os.path.join(bl.vectorsDir, 'dataFromMemory%s.bin'%''.join(random.sample(string.digits, 5)))
    status, results = bl.read_memory(fillAddress, fillLength, readDataFile)
    assert status == bootloader.status.kStatus_Success
    # Validate whether the filled data are the same as the read out data.
    verifyFillResult = True
    fileObj = file(readDataFile, 'rb')
    for i in xrange(fillLength):
        # Move the file pointer to the specific offset
        fileObj.seek(i)
        data = fileObj.read(1)
        if(ord(data) != verifyData[i%4]):
            verifyFillResult = False
            break
    fileObj.close()
    if(verifyFillResult == True):
        print("verifyFillResult = True.")
    else:
        print("verifyFillResult = False!")
    # Remove the redundant bin files.
    batch_remove(filePath=bl.vectorsDir, fileNameExtension=['*.bin'])
    return verifyFillResult

# # @brief Validate whether the data is actually written to the memory when using write-memory command.
def verify_write_memory(bl, writeAddress, writeFile):
    print("\nRead back the data from memory to verify write-memory:"),
    length = os.path.getsize(writeFile)
    readDataFile = os.path.join(bl.vectorsDir, 'dataFromMemory%s.bin'%''.join(random.sample(string.digits, 5)))
    status, results = bl.read_memory(writeAddress, length, readDataFile)
    assert status == bootloader.status.kStatus_Success
    
    verifyWriteResult = True
    readObj = file(readDataFile, 'rb')
    writeObj = file(writeFile, 'rb')
    for i in xrange(length):
        # Move the file pointer to the specific offset
        readObj.seek(i)
        writeObj.seek(i)
        if(readObj.read(1) != writeObj.read(1)):
            verifyWriteResult = False
            break
    readObj.close()
    writeObj.close()
    if(verifyWriteResult == True):
        print("verifyWriteResult = True.")
    else:
        print("verifyWriteResult = False!")
    # Remove the redundant bin files.
    batch_remove(filePath=bl.vectorsDir, fileNameExtension=['*.bin'])
    return verifyWriteResult

# # @brief Validate whether the data is actually erased when using flash-erase-region or flash-erase-all command.
def verify_flash_erase(bl, eraseAddress, eraseLength):
    print("\nRead back the data from memory to verify the erase operation:"),
    readDataFile = os.path.join(bl.vectorsDir, 'dataFromMemory%s.bin'%''.join(random.sample(string.digits, 5)))
    status, results = bl.read_memory(eraseAddress, eraseLength, readDataFile)
    assert status == bootloader.status.kStatus_Success
    
    verifyEraseResult = True
    fileObj = file(readDataFile, 'rb')
    for i in xrange(eraseLength):
        # Move the file pointer to the specific offset
        fileObj.seek(i)
        if(ord(fileObj.read(1)) != 0xFF):
            verifyEraseResult = False
            break
    fileObj.close()
    if(verifyEraseResult == True):
        print("verifyEraseResult = True.")
    else:
        print("verifyEraseResult = False!")
    # Remove the redundant bin files.
    batch_remove(filePath=bl.vectorsDir, fileNameExtension=['*.bin'])
    return verifyEraseResult

# # @brief Compare if the data in the two given binary files are the same.
def file_comparison(binFile1, binFile2):
    fileObj1 = file(binFile1, 'rb')
    data1 = fileObj1.read()
    fileObj2 = file(binFile2, 'rb')
    data2 = fileObj2.read()
    if data1 == data2:
        return True
    else:
        return False

# # @brief Flash alignment for the flash start address.
def flash_align_down(startAddr, flashSectorSize):
    return startAddr & ~(flashSectorSize - 1)

# # @brief Flash alignment for the flash end address.
def flash_align_up(endAddr, flashSectorSize):
    return (endAddr + flashSectorSize - 1) / flashSectorSize * flashSectorSize






# # @brief Verify manipulating memory with unaligned address according to given parameters
# # @parameter          bl
#                      operationType        'write-memory'          Write random file to unaligned address
#                                           'flash-erase-region'    Erase unaligned address
#                                           'fill-memory'           Fill unaligned address
#                      memType              'flash'
#                                           'ram'
def verify_manipulate_unaligned_address_according_to_parameters(bl, operationType, memType):
    availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = get_available_memory_region(bl, memType)
    alignBase = bl.target.programAlignmentSize
    if operationType == 'fill-memory':
        for addr in range(availableRegionStartAddress + 1, availableRegionStartAddress + alignBase):
            status, results = bl.fill_memory(addr, 0x100, 0xA5, 'byte')
            if memType == 'flash':
                # if the flash address is no aligned it will return FlashAlignmentError.
                assert status == bootloader.status.kStatus_FlashAlignmentError
            elif memType == 'ram':
                assert status == bootloader.status.kStatus_Success
    elif operationType == 'write-memory':
        for addr in range(availableRegionStartAddress + 1, availableRegionStartAddress + alignBase):
            status, results = bl.write_memory_with_hex_data(addr, "{{0123456789abcdef}}")
            if memType == 'flash':
                # if the flash address is no aligned it will return FlashAlignmentError.
                assert status == bootloader.status.kStatus_FlashAlignmentError
            elif memType == 'ram':
                assert status == bootloader.status.kStatus_Success
    elif operationType == "read-memory":
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        for addr in range(availableRegionStartAddress + 1, availableRegionStartAddress + alignBase):
            status, results = bl.read_memory(addr, 0x1000, readFile) # Read 4KB
            assert status == bootloader.status.kStatus_Success
    elif operationType == 'flash-erase-region':
        byteCount = alignBase
        for addr in range(availableRegionStartAddress + 1, availableRegionStartAddress + alignBase):
            status, results = bl.flash_erase_region(addr, byteCount)
            assert status == bootloader.status.kStatus_FlashAlignmentError

# # @brief Verify manipulating memory with unaligned length according to given parameters
# # @parameter          bl
#                      operationType        'write-memory'          Write random file to unaligned address
#                                           'flash-erase-region'    Erase unaligned address
#                                           'fill-memory'           Fill unaligned address
#                      memType              'flash'
#                                           'ram'
def verify_manipulate_unaligned_length_according_to_parameters(bl, operationType, memType):
    availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = get_available_memory_region(bl, memType)
    if operationType == 'flash-erase-region':
        alignBase = bl.target.eraseAlignmentSize
        for byteCount in range(1, alignBase):
            status, results = bl.flash_erase_region(availableRegionStartAddress, byteCount)
            assert status == bootloader.status.kStatus_FlashAlignmentError

# # @breif Get all the available commands for the testing target.
#  @parameter    bl        singleton instance generated in conftest.py
#                memType   'flash'    Get flash start address
#                          'ram'      Get ram start address
def get_available_commands(bl):
    print("Get all the availabe commands:"),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_AvailableCommands)
    assert status == bootloader.status.kStatus_Success
    return results[0]


# # @breif Get the memory (flash/RAM) total size.
#  @parameter    bl        singleton instance generated in conftest.py
#                memType   'flash'    Get flash size
#                          'ram'      Get ram size
#                memIndex  0, 1, 2, ... flash/ram index
def get_memory_total_size(bl, memType, memIndex=0):
    print("Get %s total size:" %(memType)),
    if memType == 'flash':
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSizeInBytes, memIndex)
        assert status == bootloader.status.kStatus_Success
        return results[0]
    elif memType == 'ram':
        status, results = bl.get_property(bootloader.properties.kPropertyTag_RAMSizeInBytes, memIndex)
        assert status == bootloader.status.kStatus_Success
        return results[0]
    else:
        print('This memory type is not defined, only support "flash" and "ram"')
        assert False

# # @breif Get the flash sector size.
def get_flash_sector_size(bl, flashIndex=0):
    print("Get flash sector size:"),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSectorSize, flashIndex)
    assert status == bootloader.status.kStatus_Success
    return results[0]


# # @breif Get the reserved memory (flash/RAM) regions and return start address, end address and address length.
#  @parameter    bl        singleton instance generated in conftest.py
#                memType   'flash'    Get reserved flash region
#                          'ram'      Get reserved ram region
def get_reserved_memory_region(bl, memType):
    status, results = bl.get_property(bootloader.properties.kPropertyTag_ReservedRegions)
    assert status == bootloader.status.kStatus_Success

    if memType == 'flash':
        reservedRegionStartAddress = results[0]
        reservedRegionEndAddress = results[1]
        if reservedRegionStartAddress == reservedRegionEndAddress:
            reservedRegionSize = 0  # ROM has no reserved flash region
        else:
            reservedRegionSize = reservedRegionEndAddress - reservedRegionStartAddress + 1
    elif memType == 'ram':
        reservedRegionStartAddress = results[2]
        reservedRegionEndAddress = results[3]
        reservedRegionSize = reservedRegionEndAddress - reservedRegionStartAddress + 1
    else:
        raise ValueError('Have no this memType! Please input "flash" or "ram"')

    return reservedRegionStartAddress, reservedRegionEndAddress, reservedRegionSize

# # @breif Get the available memory (flash/RAM) region and return start address, end address and address length.
#  @parameter    bl        singleton instance generated in conftest.py
#                memType   'flash'      Get available flash region
#                          'ram'        Get available ram region
#                memIndex  0, 1, 2, ... flash/ram index
def get_available_memory_region(bl, memType, memIndex=0):
    reservedRegionStartAddress, reservedRegionEndAddress, reservedRegionSize = get_reserved_memory_region(bl, memType)
    memoryStartAddress, memoryEndAddress = get_memory_start_end_address(bl, memType, memIndex)
    if reservedRegionSize == 0: # No flash resevered region for ROM
        availableRegionStartAddress = memoryStartAddress
        availableRegionEndAddress = memoryEndAddress
        availableRegionSize = availableRegionEndAddress - availableRegionStartAddress + 1
    else:
        if(memoryEndAddress < reservedRegionStartAddress or memoryStartAddress > reservedRegionEndAddress):
            availableRegionStartAddress = memoryStartAddress
            availableRegionEndAddress = memoryEndAddress
            availableRegionSize = availableRegionEndAddress - availableRegionStartAddress + 1
        else:            
            # In most of the time, reservedRegionStartAddress == memoryStartAddress and reservedRegionEndAddress < memoryEndAddress
            if(reservedRegionStartAddress == memoryStartAddress and reservedRegionEndAddress < memoryEndAddress):
                availableRegionStartAddress = reservedRegionEndAddress + 1
                availableRegionEndAddress = memoryEndAddress
                availableRegionSize = availableRegionEndAddress - availableRegionStartAddress + 1
            elif(reservedRegionStartAddress == memoryStartAddress and reservedRegionEndAddress == memoryEndAddress):
                availableRegionStartAddress = 0
                availableRegionEndAddress = 0
                availableRegionSize = 0     # K3S TO1.0, all the M4 ITCM SRAM regions are reserved.
    
    return availableRegionStartAddress, availableRegionEndAddress, availableRegionSize

# # @breif Get the memory (flash/RAM) start address.
#  @parameter    bl        singleton instance generated in conftest.py
#                memType   'flash'    Get flash start address
#                          'ram'      Get ram start address
#                memIndex  0, 1, 2, ... flash/ram index
def get_memory_start_address(bl, memType, memIndex=0):
    print("Get %s start address:" %(memType)),
    if memType == 'flash':
        status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashStartAddress, memIndex)
        assert status == bootloader.status.kStatus_Success
        return results[0]
    elif memType == 'ram':
        status, results = bl.get_property(bootloader.properties.kPropertyTag_RAMStartAddress, memIndex)
        assert status == bootloader.status.kStatus_Success
        return results[0]
    else:
        print('This memory type is not defined, only support "flash" and "ram"')
        assert False

# # @breif Get start address and end address of the memory (flash/RAM).
#  @parameter    bl        singleton instance generated in conftest.py
#                memType   'flash'    flash start address
#                          'ram'      ram start address
#                memIndex  0, 1, 2, ... flash/ram index
def get_memory_start_end_address(bl, memType, memIndex=0):
    memoryStartAddress = get_memory_start_address(bl, memType, memIndex)
    memoryEndAddress = memoryStartAddress + get_memory_total_size(bl, memType, memIndex) - 1
    return memoryStartAddress, memoryEndAddress




















# # @breif Get start address and length based on the given parameters.
# @param: bl             singleton instance generated in conftest.py
#         length         byte/sector
#         lengthType     'Bytes'                   'length' bytes
#                        'Sectors'                 'length' sectors
#                        'OneSectorPlusOneByte'    'length' * (1 sector + 1) bytes
#                        'HalfMemory'              'length' * half_flash_size bytes
#                        'AllMemory'               whole memory
#         memType        'flash'
#                        'ram'
#         locationType   'StartOfMemory'           startAddr = start address
#                        'EndOfMemory'             startAddr = end address - actual length + 1
def get_start_address_and_length(bl, length, lengthType, memType, locationType):
    memorySize = get_memory_total_size(bl, memType)
    flashSectorSize = get_flash_sector_size(bl)

    if lengthType == 'Bytes':
        actualLength = length
    elif lengthType == 'Sectors':
        actualLength = length * flashSectorSize
    elif lengthType == 'OneSectorPlusOneByte':
        actualLength = length * flashSectorSize
    elif lengthType == 'HalfMemory':
        actualLength = memorySize / 2
    elif lengthType == 'AllMemory':
        actualLength = memorySize
    else:
        raise ValueError('No such lengthType.')

    startAddress, endAddress = get_memory_start_end_address(bl, memType)

    if locationType == 'StartOfMemory':
        pass
    elif locationType == 'EndOfMemory':
        startAddress = startAddress + memorySize - actualLength
    else:
        raise ValueError('No such locationType.')

    return startAddress, actualLength

# @brief Get the location of a given block (startAddress ~ startAddress + actualLength -1)
# @parameter        bl                      singleton instance generated in conftest.py
#                   startAddress            start address of the block
#                   actualLength            byte count of the block
#                   memType                'flash' or 'ram'
def block_location(bl, startAddress, actualLength, memType):
    # First, check the input arguments
    if startAddress < 0 or actualLength < 0:
        return kInvalidParameter
    # Second, check the byteCount argument
    if actualLength == 0:
        return kZeroSizeBlock

    reservedRegionStartAddress, reservedRegionEndAddress, reservedRegionSize = get_reserved_memory_region(bl, memType)
    # For ROM that has no reserved flash region, the valiad flash start address could be kInvalidEndAddress
    if reservedRegionStartAddress == reservedRegionEndAddress:
        reservedRegionEndAddress = reservedRegionEndAddress - 1

    memoryStartAddress, memoryEndAddress = get_memory_start_end_address(bl, memType)
    endAddress = startAddress + actualLength - 1
    # Third, check if the blcok is in the invalid memory range or valid memory range.
    if ((startAddress >= memoryStartAddress) and (endAddress < reservedRegionStartAddress)) or \
       ((startAddress > reservedRegionEndAddress) and (endAddress <= memoryEndAddress)):
        return kValidMemoryRange
    else:
        return kInvalidMemoryRange

