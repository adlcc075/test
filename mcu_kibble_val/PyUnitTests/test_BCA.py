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
import sys
import os
import time
import random

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from fsl.bootloader import bootsources
from common import common_util
from common import user_config_area


## @breif Skip the BCA test on flashload and flashloader_loader
@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if tgt.bootloaderType == bootsources.kBootRAM_ExecuteRAM or \
       tgt.bootloaderType == bootsources.kBootFlash_ExecuteRAM:
        pytest.skip("This case is not supported on this type of bootloader.")


class TestBootloaderConfigurationArea:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)

    def test_BCA_tag(self, bl):
        # 1. Set enabledPeripherals = 0x00 in BCA to disable all the peripherals, but set the wrong tag in BCA.
        bcaFilePath = user_config_area.create_BCA_data_file(bl,
                                                       tag = user_config_area.four_char_code('k', 'c', 'f', 'G'),
                                                       enabledPeripherals = 0x00)
        # 2. Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success

        # 3. Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)

        # 4. Target can still respond to host
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        assert status == bootloader.status.kStatus_Success

    def test_BCA_enabledPeripherals(self, bl):
        # Reset target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
 
        peripherals = ['uart', 'i2c', 'spi', 'can', 'usb']
        defaultPeripheral = bl.peripheral
        # Detect the available peripheals except the default peripheral
        availablePeripherals = []
        for peripheral in peripherals:
            # Change the peripheral
            originalPeripheral = bl.change_peripheral(peripheral)
            if peripheral == defaultPeripheral:
                continue
            # Use reset command to ping the peripherals
            status, results = bl.reset()
            time.sleep(3)
            if status == bootloader.status.kStatus_Success:
                availablePeripherals.append(peripheral)
            else:
                unvailablePeripheral = bl.change_peripheral(originalPeripheral)
        print 'available peripherals: %s' %(availablePeripherals)
         
        if len(availablePeripherals) == 0:
            pytest.skip('Skip this case because only one peripheral is availabe.')
 
        # Change to the default peripheral
        originalPeripheral = bl.change_peripheral(defaultPeripheral)
 
        # Disable the default peripheral in BCA
        if defaultPeripheral == 'uart':
            bcaFilePath = user_config_area.create_BCA_data_file(bl, enabledPeripherals = 0xff - (1 << 0))
        elif defaultPeripheral == 'i2c':
            bcaFilePath = user_config_area.create_BCA_data_file(bl, enabledPeripherals = 0xff - (1 << 1))
        elif defaultPeripheral == 'spi':
            bcaFilePath = user_config_area.create_BCA_data_file(bl, enabledPeripherals = 0xff - (1 << 2))
        elif defaultPeripheral == 'can':
            bcaFilePath = user_config_area.create_BCA_data_file(bl, enabledPeripherals = 0xff - (1 << 3))
        elif defaultPeripheral == 'usb':
            bcaFilePath = user_config_area.create_BCA_data_file(bl, enabledPeripherals = 0xff - (1 << 4))
        else:
            pass
 
        # Write the BCA configuration data to flash
        status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
        assert status == bootloader.status.kStatus_Success
 
        # Reset the target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
 
        # The default peripheral should be not available now
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        assert status != bootloader.status.kStatus_Success
 
        # Change to the other available peripheral, should work fine
        originalPeripheral = bl.change_peripheral(availablePeripherals[0])
        status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
        assert status == bootloader.status.kStatus_Success
 
        # Erase the BCA region
        common_util.erase_all_available_flash_unsecure(bl)
 
        # Reset target
        status, results = bl.reset()
        assert status == bootloader.status.kStatus_Success
        time.sleep(3)
 
        # Change to the default peripheral
        originalPeripheral = bl.change_peripheral(defaultPeripheral)
 
    def test_BCA_enable_uart_while_disable_usb(self, bl):
        if bl.peripheral != 'uart':
            pytest.skip("This case is only supported on UART peripheral.")
        else:
            # Generate user configuration file with UART enabled only.
            bcaFilePath = user_config_area.create_BCA_data_file(bl, enabledPeripherals = 0x01)
            # Write the user configuration file to user configuration area
            status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
            assert status == bootloader.status.kStatus_Success
            # Reset the target
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(3)
            # Test if UART can work when USB is disabled.
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            
            # This case will fail on L5K when the UART baud rate is equal or greater than 115200bps.
            # What's more, it will cause all the other cases fail.
            # So here we use jlink to unlock the device.
            if status != bootloader.status.kStatus_Success:
                print("This case fails when UART baud rate = %sbps." %bl.speed)
                bl.target.unlock()

            assert status == bootloader.status.kStatus_Success
 
    def test_BCA_i2c_slave_address(self, bl):
        if bl.peripheral != 'i2c':
            pytest.skip("This case is only supported on I2C peripheral.")
        else:
            # Generate random data for i2cSlaveAddress
            i2cSlaveAddress = random.randint(0x00, 0xff)
 
            # Generate BCA data and write to the BCA.dat file
            bcaFilePath = user_config_area.create_BCA_data_file(bl, i2cSlaveAddress = i2cSlaveAddress)
 
            # Write the BCA configuration data to flash
            status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFilePath)
            assert status == bootloader.status.kStatus_Success
 
            # Set the I2C slave address
            bl.set_I2C_slave_address(i2cSlaveAddress)
 
            # Use the setting i2cSlaveAddress to have a test before target reset, it should fail
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            assert status != bootloader.status.kStatus_Success
 
            # Reset the target so that the BCA data can be used.
            common_util.reset_without_check(bl)
 
            # Use the setting i2cSlaveAddress to have a test after target reset, it should pass
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            assert status == bootloader.status.kStatus_Success
 
 
    # # @brief Configure usbVid and usbPid in BCA.
    def test_BCA_usb_vid_pid(self, bl):
        if bl.peripheral != 'usb':
            pytest.skip("This case is only supported on USB peripheral.")
        else:
            # Generate random data for Vid and Pid.
            Vid = random.randint(0, 0xffff)
            Pid = random.randint(0, 0xffff)
 
            # Generate BCA data and write to the BCA.dat file
            bcaFile = user_config_area.create_BCA_data_file(bl, usbVid=Vid, usbPid=Pid)
 
            # Write the BCA configuration data to flash
            status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFile)
            assert status == bootloader.status.kStatus_Success
 
            # Set the USB Vid and Pid
            bl.set_usb_vid_pid(Vid, Pid)
 
            # Use the setting Vid and Pid to have a test before target reset, it should fail.
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            assert status != bootloader.status.kStatus_Success
 
            # Reset the target so that the BCA data can be used.
            common_util.reset_without_check(bl)
            # Reserve some time to install the USB driver.
            time.sleep(30)
 
            # Use the previous Vid and Pid to have test before reset the target. (Should be pass)
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            assert status == bootloader.status.kStatus_Success
 
    def test_BCA_can_slave_txrx_id(self, bl):
        if bl.peripheral != 'can':
            pytest.skip("This case is only supported on CAN peripheral.")
        else:
            kCanDefaultSlaveTxId = 0x321
            kCanDefaultSlaveRxId = 0x123
            kCanTestSlaveTxIdList = [0x7FF, 0x322, kCanDefaultSlaveTxId, 0x001]
            kCanTestSlaveRxIdList = [0x7FE, 0x124, kCanDefaultSlaveRxId, 0x000]
            for i in range(0, len(kCanTestSlaveTxIdList)):
                for j in range(0, len(kCanTestSlaveRxIdList)):
                    # Step 1. Generate a user config file with the specified TxId and RxId
                    bcaFile = user_config_area.create_BCA_data_file(bl, canTxId = kCanTestSlaveTxIdList[i], canRxId = kCanTestSlaveRxIdList[j])
                    # Step 2. Write the BCA configuration data to flash
                    status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFile)
                    assert status == bootloader.status.kStatus_Success
 
                    # Step 3. Reset the target so that the BCA data can be used.
                    status, results = bl.reset()
                    assert status == bootloader.status.kStatus_Success
                    time.sleep(3)
 
                    # Step 4. Change the can TxId and RxId after BCA taking effect. Should be OK.
                    bl.set_CAN_parameters(bl.speed, kCanTestSlaveRxIdList[j], kCanTestSlaveTxIdList[i])
                    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
                    assert status == bootloader.status.kStatus_Success
 
                    # Step 5. Chang the can TxId and RxId again.
                    #        Actually, the TxId and RxId cannot be changed again,
                    #        otherwise, target should not send response to host
                    k = random.randint(0, len(kCanTestSlaveTxIdList) - 1)
                    l = random.randint(0, len(kCanTestSlaveRxIdList) - 1)
                    bl.set_CAN_parameters(bl.speed, kCanTestSlaveRxIdList[l], kCanTestSlaveTxIdList[k])
                    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
                    if i == k and j == l:
                        assert status == bootloader.status.kStatus_Success
                    else:
                        assert status != bootloader.status.kStatus_Success
                        # Step 6. Set back the correct TxId and RxId, so that it can work properly.
                        bl.set_CAN_parameters(bl.speed, kCanTestSlaveRxIdList[j], kCanTestSlaveTxIdList[i])
                        time.sleep(3)
                    # Step 7. Erase the flash including the BCA field, so that Step 2 will not encounter kStatus_FlashCommandFailure.
                    common_util.erase_all_available_flash_unsecure(bl)
            # Reset the target to clean the test environment.
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(3)
            # Set back the default TxId and RxId
            bl.set_CAN_parameters(bl.speed, kCanDefaultSlaveTxId, kCanDefaultSlaveRxId)
 
    def test_BCA_can_speed_index(self, bl):
        if bl.peripheral != 'can':
            pytest.skip("This case is only supported on CAN peripheral.")
        else:
            kCanUserDefinedSpeedList = [0xF0, 0xF1, 0xF2, 0xF4]
            kSupportedPeripheralSpeed_can = [0, 1, 2, 4]    # 125k, 250k, 500k, 1M
            for i in range(0, len(kCanUserDefinedSpeedList)):
                # Step 1. Generate a user config file with pre-defined can speed
                bcaFile = user_config_area.create_BCA_data_file(bl, canConfig1 = kCanUserDefinedSpeedList[i])
 
                # Step 2. Write the BCA configuration data to flash
                status, results = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS + 0x3C0, bcaFile)
                assert status == bootloader.status.kStatus_Success
 
                # Step 3. Reset the target so that the BCA data can be used.
                status, results = bl.reset()
                assert status == bootloader.status.kStatus_Success
                time.sleep(3)
 
                # Step 4. Change the can speed after BCA taking effect. Should be OK.
                bl.set_CAN_parameters(kSupportedPeripheralSpeed_can[i])
                status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
                assert status == bootloader.status.kStatus_Success
 
                for j in range(0, len(kCanUserDefinedSpeedList)):
                    # Step 5. Change the can speed again.
                    #        If the two speeds are equal, target should send response to host,
                    #        otherwise, target should not send response to host
                    bl.set_CAN_parameters(kSupportedPeripheralSpeed_can[j])
                    status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
                    if j == i:
                        assert status == bootloader.status.kStatus_Success
                    else:
                        assert status != bootloader.status.kStatus_Success
                        time.sleep(3)
                # Step 6. Set back the correct can speed, so that it can work properly.
                bl.set_CAN_parameters(kSupportedPeripheralSpeed_can[i])
                # Step 7. Erase the flash including the BCA field, so that Step 2 will not encounter kStatus_FlashCommandFailure.
                common_util.erase_all_available_flash_unsecure(bl)
            # Reset the target to clean the test environment.
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(3)