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
import copy
import subprocess
import json
import math

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '../..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from PyUnitTests import bltest_config


#
kBuspalUartDefaultSpeed = 57600
kDefaultI2cSlaveAddress = 0x10
kDefulatCanSlaveTxRxId = '0x321,0x123'

# Errors returned by the Bootloader command methods.
kBlhostError_NoOutput = -3
kBlhostError_ReturnedError = -4

class Bootloader(object):
    def __init__(self, target, vectorsDir):

        self._commandArgs = []

        self.target = target

        # Path to directory containing test vectors.
        # This path is also used to find blhost tool.
        self.vectorsDir = vectorsDir

        # This is a string with one of the peripheral names.
        self.peripheral = ''

        # The full output of the last execution of blhost as a string
        self.commandOutput = ""

        # A dictionary of the response from the bootloader for the last command execution.
        self.commandResults = {}

        # Status code returned by the blhost process the last time it was run.
        self.toolStatus = 0

        # The size will use to calculate timeout value of waiting response of blhost
        self.size = 0

        # timeout value for waiting return value from blhost, the default value is 5000ms.
        self.timeout = 5000
        

    def close(self):
        pass

    # @brief  A utility function to return the JSON formatted results.
    def _parse_results(self, output):
        if self.toolStatus == 0:
            jsonResults = output[output.find('{') : output.rfind('}') + 1]
            if len(jsonResults):
                actualResults = json.loads(jsonResults)
                return actualResults

            # No json was found, create artificial results.
            actualResults = {
                                'status':   {
                                                'value': kBlhostError_NoOutput,
                                                'description': 'The command did not return a status.'
                                            },
                                'response': None
                            }
        else:  # self.toolStatus != 0, it means blhost returned error code.
            actualResults = {
                                'status':   {
                                                'value': kBlhostError_ReturnedError,
                                                'description': 'The command returned error %d.' % self.toolStatus
                                            },
                                 'response': None
                            }
        return actualResults

    # @brief  A utility function to run executable blhost for code reuse.
    # @return A bi-tuple of the command status, and the dictionary of command results.
    def _execute_command(self, *args):
        # Make a copy of the base args so we don't mess up the original.
        theArgs = copy.copy(self._commandArgs)
        # Set timeout value for waiting results from blhost        
        timeoutArgument = self.get_timeout_value(args)
        theArgs.extend(timeoutArgument)
        theArgs.extend(['-j', '--'])
        theArgs.extend(args)

        # theArgs is list type, first to convert its inner elements to string type.
        theArgs = [str(x) for x in theArgs]
        # Use a SPACE to connect every inner element in theArgs and convert theArgs from list type to string type.
        print("\nExecuting: blhost %s" %" ".join(theArgs[1:]))
        theArgs = " ".join(theArgs)
        # print "\nExecuting:", theArgs

        # Execute the command.
        # The first arg of subprocess.Popen must be string, that's why we convert theArgs to string type.
        process = subprocess.Popen(theArgs, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        self.commandOutput = process.communicate()[0]
        self.toolStatus = process.returncode

        print 'toolStatus:', self.toolStatus
        print 'commandOutput:', self.commandOutput,

        # Convert command JSON output into a dict.
        self.commandResults = self._parse_results(self.commandOutput);

        # Set some attributes based on results dictionary.
        return self.commandResults['status']['value'], self.commandResults['response']

    # @brief Set timeout value for waiting results from blhost
    ''' 
    Note: The meaning of the timeout parameter in the blhost command has been changed from KBL 2.0.
    The timeout value means how long it takes to finish the entire blhost command in the previous blhost
    version, but now it means how long it takes for host to receive the response when sending one data package.
    Each blhost command is composed of one or more data packages.

    This function is designed based on the blhost version in KBL 2.0 RC4 release.
    Below are the testing results on FRDM-K66 based on the KBL 2.0 RC4 release.
    ____________________________________________________________________________________________________________
    |blhost command    |peripheral |speed(bps)  |flash size  |timeout value(ms) |Is timeout impacted by speed?
    ------------------------------------------------------------------------------------------------------------
    |flash-erase-region|UART       |4800,115200 |1MB         |2000,2000         |No, almost not.
    ------------------------------------------------------------------------------------------------------------
    |fill-memory       |UART       |4800,115200 |1MB         |25000,25000       |No, almost not.
    ------------------------------------------------------------------------------------------------------------
    |write-memory      |UART       |4800,115200 |1MB         |40000,25000       |Yes, a little.
    ------------------------------------------------------------------------------------------------------------
    |read-memory       |UART       |4800,115200 |1MB         |2,2               |No, almost not.
    ------------------------------------------------------------------------------------------------------------
    |receive-sb-file   |UART       |4800,115200 |1MB         |Not test          |
    ------------------------------------------------------------------------------------------------------------
    |get-property      |UART       |4800,115200 |1MB         |1,1               |No, almost not.
    ------------------------------------------------------------------------------------------------------------
    '''
    def get_timeout_value(self, cmdArgs):            
        if self.peripheral == 'uart':
            # supportedPeripheralSpeed_uart = [4800, 9600, 19200, 57600, 115200]
            multiplyFactor = 48000/(int(self.speed)/4800)
        elif self.peripheral == 'i2c' or self.peripheral == 'spi':
            # supportedPeripheralSpeed_i2c = [5, 50, 100, 400]
            # supportedPeripheralSpeed_spi = [5, 10, 20, 50, 100, 200, 500]
            j = math.log(int(self.speed), 2)
            multiplyFactor = int((0.65**j)*200000)
        elif self.peripheral == 'can':
            # supportedPeripheralSpeed_can = [0, 1, 2, 4]# '0' means 125k, '1' means 250k, '2' means 500k, '4' means 1M
            j = (0.5**(int(self.speed)))*10000
            multiplyFactor = int(math.log(j,30)*1000)
        elif self.peripheral == 'usb':
            multiplyFactor = 1000

        size16KB    = 0x4000 
        size32KB    = 0x8000
        size64KB    = 0x10000
        size128KB   = 0x20000
        size256KB   = 0x40000
        size512KB   = 0x80000
        size1MB     = 0x100000
        size2MB     = 0x200000
        size4MB     = 0x400000
        size8MB     = 0x800000
        size16MB    = 0x1000000
        if ('flash-erase-region' in cmdArgs) or ('flash-erase-all' in cmdArgs):
            # 1MB <--> 2s (internal flash) ==> 0x80000 Byte/s = 512 KB/s
            # 1MB <--> 7s (qspi     flash) ==> 0x24924 Byte/s > 128 KB/s
            self.timeout = 5000 + 1000*(self.size/size128KB)
        elif 'fill-memory' in cmdArgs:
            # 1MB <--> 25s ==> 0xA3D7 Byte/s > 32 KB/s
            # 1MB <--> 30s ==> 0x8888 Byte/s > 32 KB/s
            self.timeout = 5000 + multiplyFactor*(self.size/size32KB)*10
        
        elif 'write-memory' in cmdArgs:
            # 1MB <--> 40s ==> 0x6666 Byte/s > 32 KB/s
            self.timeout = 5000 + multiplyFactor*(self.size/size32KB)
        elif 'read-memory' in cmdArgs:
            self.timeout = 5000 + 1000*(self.size/size32KB)
        elif 'receive-sb-file' in cmdArgs:
            self.timeout = 400000 # don't know what is in SB file, so give it a long time
        else:
            self.timeout = 5000   # default value
    
        return ['-t', str(self.timeout)]

    # blhost command 1: get-property
    def get_property(self, tag, memoryID = ''):
        # Property 25 is for QSPI Flash Memory Attrubutes, and memoryId is required.
        return self._execute_command('get-property', tag, memoryID)

    # blhost command 2: set-property
    def set_property(self, tag, value):
        return self._execute_command('set-property', tag, value)

    # blhost command 3: flash-erase-region
    # This command erases a region of flash
    def flash_erase_region(self, address, byteCount):
        self.size = byteCount
        return self._execute_command('flash-erase-region', hex(address).split('L')[0], hex(byteCount).split('L')[0])

    # blhost command 4: flash-erase-all
    # This command erases all the flash, excluding protected regions.
    def flash_erase_all(self, memoryID = 0):
        if memoryID == 0:
            self.size = 0x200000   # 2MB
        elif memoryID == 1:
            self.size = 0x1000000  # 16MB
        return self._execute_command('flash-erase-all', memoryID)

    # blhost command 5: flash-erase-all-unsecure
    # This command erases all the flash, including protected regions.
    # This command is not supported for flash-resident bootloader, it's for ROM-resident bootloader.
    def flash_erase_all_unsecure(self):
        return self._execute_command('flash-erase-all-unsecure')

    # blhost command 6: read-memory
    def read_memory(self, address, byteCount, fileName):
#         # Open the read_data_from_memory.dat file.
#         # If not exist then create a new file named read_data_from_memory.bin.
#         fileHandle = open(fileName, 'w')
#         fileHandle.truncate()
#         # Close the opened file.
#         fileHandle.close()
        self.size = byteCount
        return self._execute_command('read-memory', hex(address).split('L')[0], hex(byteCount).split('L')[0], fileName)

    # blhost command 7: write-memory
    def write_memory(self, address, fileName):
        self.size = os.path.getsize(fileName)
        return self._execute_command('write-memory', hex(address).split('L')[0], fileName)

    # blhost command 7.1: write-memory
    def write_memory_with_hex_data(self, address, hexData):
        self.size = len(hexData)
        return self._execute_command('write-memory', hex(address).split('L')[0], hexData)

    # blhost command 8: fill-memory
    def fill_memory(self, address, byteCount, pattern, unit='word'):
        self.size = byteCount
        return self._execute_command('fill-memory', hex(address).split('L')[0], hex(byteCount).split('L')[0], hex(pattern).split('L')[0], unit)

    # blhost command 9: execute
    def execute(self, address, arg, stackPointer):
        return self._execute_command('execute', hex(address).split('L')[0], arg, hex(stackPointer).split('L')[0])

    # blhost command 10: call
    def call(self, address, arg):
        return self._execute_command('call', hex(address).split('L')[0], arg)

    # blhost command 11: flash-security-disable
    def flash_security_disable(self, key):
        return self._execute_command('flash-security-disable', key)

    # blhost command 12: flash-program-once
    def flash_program_once(self, index, byte_count, data, endian = 'lsb'):
        return self._execute_command('flash-program-once', hex(index).split('L')[0], byte_count, data, endian)

    # blhost command 13: flash-read-once
    def flash_read_once(self, index, byte_count):
        return self._execute_command('flash-read-once', hex(index).split('L')[0], byte_count)

    # blhost command 14: flash-read-resource
    def flash_read_resource(self, address, byte_count, option, filename):
        pass

    # blhost command 15: configure-memory
    def configure_memory(self, memoryId, address):
        return self._execute_command('configure-memory', memoryId, hex(address).split('L')[0])

    # blhost command 16: reset
    def reset(self):
        return self._execute_command('reset')

    # blhost command 17: receive-sb-file
    def receive_sb_file(self, sbFilePath):
        return self._execute_command('receive-sb-file', sbFilePath)

    # blhost command 18: reliable-update
    def reliable_update(self, address):
        return self._execute_command('reliable-update', hex(address).split('L')[0])

class BootloaderDevice(Bootloader):
    def __init__(self, target, vectorsDir, peripheral, speed, port, loadTarget, resetTarget, usePing):

        # Now self._commandArgs is []
        super(BootloaderDevice, self).__init__(target, vectorsDir)  # Call the init function of the parent class

        self._port = port
        self._loadTarget = loadTarget
        self._resetTarget = resetTarget
        self._usePing = usePing
        self.peripheral = peripheral
        self.speed = speed
        
        # Generate blhost command args
        self._commandArgs = self.generate_cmd_Args()

    # @brief Generate the blhost command args according to the peripheral.
    def generate_cmd_Args(self):
        # _toolName is ['..\\working\\blhost']
        toolName = os.path.abspath(os.path.join(self.vectorsDir, '..', 'blhost'))

        if self.peripheral == 'usb':  # ..\working\blhost -u
            cmdArgs = [toolName,
                       '-u']
        elif self.peripheral == 'uart':
            # ..\working\blhost -p COMx,kUartDefaultSpeed
            cmdArgs = [toolName,
                       '-p',
                       self._port + ',' + self.speed]

        elif self.peripheral == 'i2c':
            cmdArgs = [toolName,
                           '-p',
                            self._port + ',' + str(kBuspalUartDefaultSpeed),
                            '-b',
                            self.peripheral + ',' + hex(kDefaultI2cSlaveAddress) + ',' + self.speed]
        elif self.peripheral == 'spi':
            cmdArgs = [toolName,
                           '-p',
                            self._port + ',' + str(kBuspalUartDefaultSpeed),
                            '-b',
                            self.peripheral + ',' + self.speed]
        elif self.peripheral == 'can':
            cmdArgs = [toolName,
                           '-p',
                            self._port + ',' + str(kBuspalUartDefaultSpeed),
                            '-b',
                            self.peripheral + ',' + self.speed + ',' + kDefulatCanSlaveTxRxId]
        else:
            raise ValueError('Invalid peripheral parameter: %s' % peripheral)

        if self.peripheral == 'i2c' or self.peripheral == 'spi' or self.peripheral == 'can':
            if not self._usePing:
                cmdArgs.extend(['-n'])
            else:
                pass
        else:
            pass

        return cmdArgs

    # @brief Change to the specific peripheral and return the original peripheral.
    def change_peripheral(self, peripheral):
        originalPeripheral = self.peripheral
        self.peripheral = peripheral
        self._port = bltest_config.peripheralParamsDict[peripheral].port
        self.speed = bltest_config.peripheralParamsDict[peripheral].speed
        self._commandArgs = self.generate_cmd_Args()
        return originalPeripheral

    # @brief Set UART baud rate
    def set_uart_baud_rate(self, uartBaudRate):
        if self.peripheral == 'uart':
            self.speed = uartBaudRate
            # self._commandArgs = ['..\\blhost', '-p', 'COM131,57600', '-t', '50000', '-j', '--']            
            uartBaudRateIndex = self._commandArgs.index('-p') + 1
            uartPortBaudRate = self._commandArgs[uartBaudRateIndex].split(',')
            
            if len(uartPortBaudRate) > 1:
                uartPortBaudRate[1] = str(uartBaudRate)
            else:
                uartPortBaudRate.append(str(uartBaudRate))
            self._commandArgs[uartBaudRateIndex] = ','.join(uartPortBaudRate)
        else:
            raise ValueError('Requires a UART peripheral.')

    # @brief Set I2C slave address
    def set_I2C_slave_address(self, i2cSlaveAddress):
        if self.peripheral == 'i2c':
            i2cSlaveAddressIndex = self._commandArgs.index('-b') + 1
            i2cSlaveAddressAndSpeed = self._commandArgs[i2cSlaveAddressIndex].split(',')
            i2cSlaveAddressAndSpeed[1] = hex(i2cSlaveAddress)
            self._commandArgs[i2cSlaveAddressIndex] = ','.join(i2cSlaveAddressAndSpeed)
        else:
            raise ValueError('Requires an I2C peripheral.')

    # @brief Set speed for I2C peripheral
    def set_I2C_speed(self, i2cSpeed):
        if self.peripheral == 'i2c':
            self.speed = i2cSpeed
            i2cSpeedIndex = self._commandArgs.index('-b') + 1
            i2cSlaveAddressAndSpeed = self._commandArgs[i2cSpeedIndex].split(',')
            i2cSlaveAddressAndSpeed[2] = str(i2cSpeed)
            self._commandArgs[i2cSpeedIndex] = ','.join(i2cSlaveAddressAndSpeed)
        else:
            raise ValueError('Requires an I2C peripheral.')

    # @brief Set speed for SPI peripheral
    def set_SPI_speed(self, spiSpeed):
        if self.peripheral == 'spi':
            self.speed = spiSpeed
            spiSpeedIndex = self._commandArgs.index('-b') + 1
            spiComSpeed = self._commandArgs[spiSpeedIndex].split(',')
            spiComSpeed[1] = str(spiSpeed)
            self._commandArgs[spiSpeedIndex] = ','.join(spiComSpeed)
        else:
            raise ValueError('Requires an SPI peripheral.')

    # @brief Set speed, TX ID and RX ID for FlexCan peripheral
    def set_CAN_parameters(self, canSpeed, TxId = 0x321, RxId = 0x123):
        if self.peripheral == 'can':
            self.speed = canSpeed
            canParametersIndex = self._commandArgs.index('-b') + 1
            canParameters = self._commandArgs[canParametersIndex].split(',')    # canParameters = ['can', '4', '0x123', '0x321']
            canParameters[1] = str(canSpeed)
            canParameters[2] = hex(TxId)
            canParameters[3] = hex(RxId)
            self._commandArgs[canParametersIndex] = ','.join(canParameters)
        else:
            raise ValueError('Requires a CAN peripheral.')

    # @brief Set USB Vid and Pid parameters.
    def set_usb_vid_pid(self, Vid, Pid):
        if self.peripheral == 'usb':
            # self._commandArgs = ['..\\blhost', '-u', '-t', '50000', '-j', '--']
            usbVidPidIndex = self._commandArgs.index('-u') + 1

            VidPid = [hex(Vid), hex(Pid)]
            self._commandArgs.insert(usbVidPidIndex, ','.join(VidPid))
        else:
            raise ValueError('Requires a USB peripheral.')