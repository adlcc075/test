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
import time

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util

# @breif Generate the BCA tag.
def four_char_code(a, b, c, d):
    return ((ord(d) << 24) | (ord(c) << 16) | (ord(b) << 8) | (ord(a)))

# @breif Write 8/16/32 bits data to bytearray.
def write_byte_array(byteArray, data, dataBits):
    if(dataBits >= 8):
        byteArray.append(data & 0xFF)
    if(dataBits >= 16):
        byteArray.append((data >> 8) & 0xFF)
    if(dataBits >= 32):
        byteArray.append((data >> 16) & 0xFF)
        byteArray.append((data >> 24) & 0xFF)
    return byteArray

# @breif Generate the BCA data.
def create_BCA_data_file(
                       bl,
                       tag                          =   four_char_code('k', 'c', 'f', 'g'),
                       crcStartAddress              =   0xFFFFFFFF,
                       crcByteCount                 =   0xFFFFFFFF,
                       crcExpectedValue             =   0xFFFFFFFF,
                       enabledPeripherals           =   0xFF,
                       i2cSlaveAddress              =   0xFF,
                       peripheralDetectionTimeout   =   0xFFFF,
                       usbVid                       =   0xFFFF,
                       usbPid                       =   0xFFFF,
                       usbStringsPointer            =   0xFFFFFFFF,
                       clockFlags                   =   0xFF,
                       clockDivider                 =   0xFF,
                       bootFlags                    =   0xFF,
                       pad0                         =   0xFF,
                       mmcauConfigPointer           =   0xFFFFFFFF,
                       keyBlobPointer               =   0xFFFFFFFF,
                       pad1                         =   0xFF,
                       canConfig1                   =   0xFF,
                       canConfig2                   =   0xFFFF,
                       canTxId                      =   0xFFFF,
                       canRxId                      =   0xFFFF,
                       qspiConfigBlockPointer       =   0xFFFFFFFF
                    ):

    bcaData = bytearray()
    # Write BCA data to bytearray
    bcaData = write_byte_array(bcaData, tag,                        32)
    bcaData = write_byte_array(bcaData, crcStartAddress,            32)
    bcaData = write_byte_array(bcaData, crcByteCount,               32)
    bcaData = write_byte_array(bcaData, crcExpectedValue,           32)
    bcaData = write_byte_array(bcaData, enabledPeripherals,          8)
    bcaData = write_byte_array(bcaData, i2cSlaveAddress,             8)
    bcaData = write_byte_array(bcaData, peripheralDetectionTimeout, 16)
    bcaData = write_byte_array(bcaData, usbVid,                     16)
    bcaData = write_byte_array(bcaData, usbPid,                     16)
    bcaData = write_byte_array(bcaData, usbStringsPointer,          32)
    bcaData = write_byte_array(bcaData, clockFlags,                  8)
    bcaData = write_byte_array(bcaData, clockDivider,                8)
    bcaData = write_byte_array(bcaData, bootFlags,                   8)
    bcaData = write_byte_array(bcaData, pad0,                        8)
    bcaData = write_byte_array(bcaData, mmcauConfigPointer,         32)
    bcaData = write_byte_array(bcaData, keyBlobPointer,             32)
    bcaData = write_byte_array(bcaData, pad1,                        8)
    bcaData = write_byte_array(bcaData, canConfig1,                  8)
    bcaData = write_byte_array(bcaData, canConfig2,                 16)
    bcaData = write_byte_array(bcaData, canTxId,                    16)
    bcaData = write_byte_array(bcaData, canRxId,                    16)
    bcaData = write_byte_array(bcaData, qspiConfigBlockPointer,     32)

    # Write the BCA data bytes to the BCA.dat file
    bcaFilePath = os.path.join(bl.vectorsDir, 'BCA.dat')
    fileObj = open(bcaFilePath, 'w+')
    fileObj.write(bcaData)
    fileObj.close()
    return bcaFilePath

########################################################################################
#
# Test bootloader configuration area (BCA)
#
########################################################################################
def get_BCA_location(bl, bootCore):
    if(bootCore == 'core0'):
        bcaLocation = bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0
    elif(bootCore == 'core1'):
        flash1StartAddress = common_util.get_memory_start_address(bl, 'flash', 1)
        bcaLocation = flash1StartAddress + 0x3C0
    print("BCA location address = 0x%x"%bcaLocation)
    return bcaLocation

def BCA_tag_not_kcfg(bl, bootCore):
    bcaLocation = get_BCA_location(bl, bootCore)
    print("\nWrite BCA data to BCA location 0x%x:"%bcaLocation)
    print("1) Disable BCA (tag != 'kcfg');")
    print("2) Disable all the peripherals (enabledPeripherals = 0x00)."),
    bcaFilePath = create_BCA_data_file(bl,
                                       tag = four_char_code('k', 'c', 'f', 'G'),
                                       enabledPeripherals = 0x00)
    status, results = bl.write_memory(bcaLocation, bcaFilePath)
    assert status == bootloader.status.kStatus_Success
        
    print("\nReset target so that the BCA data can be used:"),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    time.sleep(3)
    
    print("\nTarget still can talk to the host by the peripheral because BCA is disabled (tag != 'kcfg')."),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    assert status == bootloader.status.kStatus_Success

def BCA_disable_peripheral(bl, bootCore, peripheral):
    if(peripheral.upper() == 'UART'):
        enabledPeripherals = 0xFF ^ (1 << 0)
    elif(peripheral.upper() == 'I2C'):
        enabledPeripherals = 0xFF ^ (1 << 1)
    elif(peripheral.upper() == 'SPI'):
        enabledPeripherals = 0xFF ^ (1 << 2)
    elif(peripheral.upper() == 'CAN'):
        enabledPeripherals = 0xFF ^ (1 << 3)
    elif(peripheral.upper() == 'USB'):
        enabledPeripherals = 0xFF ^ (1 << 4)

    bcaLocation = get_BCA_location(bl, bootCore)
    print("\nWrite BCA data to BCA location 0x%x:"%bcaLocation)
    print("1) Set BCA tag as 'kcfg';")
    print("2) Disable %s peripheral (enabledPeripherals = 0x%x)."%(peripheral.upper(), enabledPeripherals)),
    bcaFilePath = create_BCA_data_file(bl, enabledPeripherals = enabledPeripherals)    
    status, results = bl.write_memory(bcaLocation, bcaFilePath)
    assert status == bootloader.status.kStatus_Success
        
    print("\nReset target so that the BCA data can be used:"),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    time.sleep(3)
    
    if(bl.peripheral == peripheral):
        print("\nTarget cannot talk to the host by %s because this peripheral is disabled in BCA."%(peripheral.upper())),
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        assert status != bootloader.status.kStatus_Success
    # UART is enabled while USB is diabled in the BCA, UART should work fine.
    elif(bl.peripheral.upper() == 'UART' and peripheral.upper() == 'USB'):
        print("\nCurrent working peripheral is %s, "%bl.peripheral.upper()),
        print("while %s is disabled in BCA, %s should work fine."%(peripheral.upper(), bl.peripheral.upper())),
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        assert status == bootloader.status.kStatus_Success
        
def BCA_i2c_slave_address(bl, bootCore):
    # Generate random data from 0x00 to 0xFE for i2cSlaveAddress
    i2cSlaveAddress = random.randint(0x00, 0xfe)
    # Make sure the setting i2c slave address is not the default value 0x10.
    while(i2cSlaveAddress == 0x10):
        i2cSlaveAddress = random.randint(0x00, 0xfe)
        
    bcaLocation = get_BCA_location(bl, bootCore)
    print("\nWrite BCA data to BCA location 0x%x:"%bcaLocation)
    print("1) Set BCA tag as 'kcfg';")
    print("2) Set I2C slave address as 0x%x."%(i2cSlaveAddress)),
    bcaFilePath = create_BCA_data_file(bl, i2cSlaveAddress = i2cSlaveAddress)    
    status, results = bl.write_memory(bcaLocation, bcaFilePath)
    assert status == bootloader.status.kStatus_Success
        
    print("\nReset target so that the BCA data can be used:"),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    
    print("\nUse the default i2c slave address 0x10 to communicate with host, it should fail."),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    assert status != bootloader.status.kStatus_Success
    
    print("\nChange the I2C slave address as the setting value 0x%x."%i2cSlaveAddress)
    bl.set_I2C_slave_address(i2cSlaveAddress)
    print("Then use the slave address 0x%x to communicate with host, it should work fine."%i2cSlaveAddress),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    assert status == bootloader.status.kStatus_Success
    
    
def BCA_usb_vid_pid(bl, bootCore):
    # Generate random data for vid and pid.
    vid = random.randint(0, 0xfffe)
    pid = random.randint(0, 0xfffe)
    # Make sure not all the setting vid and pid are the default values 
    while(vid == 0x15a2 and pid == 0x0073):
        vid = random.randint(0, 0xfffe)
        pid = random.randint(0, 0xfffe)
    bcaLocation = get_BCA_location(bl, bootCore)
    print("\nWrite BCA data to BCA location 0x%x:"%bcaLocation)
    print("1) Set BCA tag as 'kcfg';")
    print("2) Set USB Vid and Pid as 0x%x, 0x%x."%(vid, pid)),
    bcaFilePath = create_BCA_data_file(bl, usbVid = vid, usbPid = pid)    
    status, results = bl.write_memory(bcaLocation, bcaFilePath)
    assert status == bootloader.status.kStatus_Success
        
    print("\nReset target so that the BCA data can be used:"),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    print("Need reserve some time here for PC to install the USB driver...")
    time.sleep(30)
    
    print("\nUse the default USB Vid and Pid to communicate with host, it should fail."),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    assert status != bootloader.status.kStatus_Success
    print("Change the USB Vid and Pid as the setting values 0x%x, 0x%x."%(vid, pid))
    bl.set_usb_vid_pid(Vid, Pid)
    print("Then use the setting USB Vid and Pid to communicate with host, it should work fine."),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    assert status == bootloader.status.kStatus_Success

def BCA_can_slave_txrx_id(bl, bootCore):
    # Generate random data for canTxId and canRxId.
    txId = random.randint(0, 0xfffe)
    rxId = random.randint(0, 0xfffe)
    # Make sure not all the setting canTxId and canRxId are the default values 
    while( ((txId & 0x7FF) == 0x321) and ((rxId & 0x7FF) == 0x123)  ):
        txId = random.randint(0, 0xfffe)
        rxId = random.randint(0, 0xfffe)
    
    bcaLocation = get_BCA_location(bl, bootCore)
    print("\nWrite BCA data to BCA location 0x%x:"%bcaLocation)
    print("1) Set BCA tag = 'kcfg';")
    print("2) Set canTxId = 0x%x, canRxId = 0x%x in BCA."%(txId, rxId))
    print("NOTE: FlexCAN only supports 11-bit canTxId and 11-bit canRxId, so actually,")
    print("      canTxId = (0x%x & 0x7ff) = 0x%x,"%(txId, txId & 0x7FF))
    print("      canRxId = (0x%x & 0x7ff) = 0x%x."%(rxId, rxId & 0x7FF)),
    
    bcaFilePath = create_BCA_data_file(bl, canTxId = txId, canRxId = rxId)    
    status, results = bl.write_memory(bcaLocation, bcaFilePath)
    assert status == bootloader.status.kStatus_Success
    
    print("\nReset target so that the BCA data can be used:"),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    time.sleep(1)
    
    print("Change the canTxId and canRxId as the setting values 0x%x, 0x%x."%(txId & 0x7FF, rxId & 0x7FF))
    bl.set_CAN_parameters(bl.speed, rxId & 0x7FF, txId & 0x7FF)
    print("Then use the setting canTxId and canRxId to communicate with host, it should work fine."),
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    assert status == bootloader.status.kStatus_Success
    
    print("\nChange the canTxId and canRxId again, target will not send any response to host."), 
    txIdTmp = random.randint(0, 0xfffe) & 0x7ff
    rxIdTmp = random.randint(0, 0xfffe) & 0x7ff
    while( ((txId & 0x7FF) == txIdTmp) and ((rxId & 0x7FF) == rxIdTmp)  ):
        txIdTmp = random.randint(0, 0xfffe) & 0x7ff
        rxIdTmp = random.randint(0, 0xfffe) & 0x7ff
    bl.set_CAN_parameters(bl.speed, rxIdTmp, txIdTmp)
    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
    assert status != bootloader.status.kStatus_Success

def BCA_set_canConfig1_canConfig2(bl, bootCore, canConfig1, canConfig2):
    bcaLocation = get_BCA_location(bl, bootCore)
    print("\nWrite BCA data to BCA location 0x%x:"%bcaLocation)
    print("1) Set BCA tag = 'kcfg';")
    print("2) Set canConfig1 = 0x%x, canConfig2 = 0x%x in BCA."%(canConfig1, canConfig2)),
    bcaFilePath = create_BCA_data_file(bl, canTxId = txId, canRxId = rxId)    
    status, results = bl.write_memory(bcaLocation, bcaFilePath)
    assert status == bootloader.status.kStatus_Success
    
    print("\nReset target so that the BCA data can be used:"),
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    time.sleep(1)
    kSupportedPeripheralSpeed_can = [0, 1, 2, 4]    # 125k, 250k, 500k, 1M
    kCanSpeedInfo  = ['125kbps', '250kbps', '500kbps', '1Mbps']
    for index, canSpeed in enumerate(kSupportedPeripheralSpeed_can):
        print("\nChange the can speed to %s" %(kCanSpeedInfo[index])),
        bl.set_CAN_parameters(canSpeed)
        
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        if(canSpeed == (canConfig1 & 0x0F)):
            assert status == bootloader.status.kStatus_Success
        else:
            assert status != bootloader.status.kStatus_Success
        print("NOTE: When canConfig1 = 0x%x, canConfig2 = 0xFFFF, FlexCAN only works on %s." %(canConfig1, kCanSpeedInfo[index]))

    
    
    
    
    
    