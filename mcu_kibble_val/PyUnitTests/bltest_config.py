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
import ConfigParser
from collections import namedtuple

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env
from fsl.bootloader import devices


# This is a test configuration file.
releasePackagePath = 'E:\\MyTesting\\Kibble_test\\Kibble2.0\\Kinetis_Bootloader_2_0_rc4'
releasePackage = os.environ.get('KIBBLE_RELEASE_PACKAGE', releasePackagePath)
##
# @brief Define which target to test.
#
# projectName has the following optional values:
# 1. 'rom_bootloader_fpga'      : ROM validation on the fpga board (before tape out)
# 2. 'rom_bootloader_tower'     : ROM validation on the tower board (after tape out)
# 3. 'rom_bootloader_freedom'   : ROM validation on the freedom board (after tape out)
# ----------------------------------------------------------------------------------------------------
# 4. 'tower_bootloader'         : Validate flash-resident bootloader on tower board
# 5. 'freedom_bootloader'       : Validate flash-resident bootloader on freedom board
# ----------------------------------------------------------------------------------------------------
# 6. 'flashloader'              : Validate ram-resident bootloader (boot from ram and execute in ram)
# 7. 'flashloader_loader'       : Validate ram-resident bootloader (boot from flash and execute in ram)
projectName     = os.environ.get("KIBBLE_PROJECT_NAME", "rom_bootloader_tower")
targetName      = os.environ.get("KIBBLE_TARGET_NAME", "K3S") # K80F25615
buildVersion    = os.environ.get("KIBBLE_BUILD_VERSION", "Release")
compiler        = os.environ.get("KIBBLE_COMPILER", "iar")  # iar, keil, kds, ...
# Providing the elf image file is very important especially for the flashloader test. 
imageElfFile    = os.environ.get('KIBBLE_IMAGE_FILE', '')
print("\nbinary file: %s\n" %imageElfFile)

targetUartPort = os.environ.get('KIBBLE_TARGET_UART_PORT', 'COM70')
buspalUartPort = os.environ.get('KIBBLE_BUSPAL_UART_PORT', 'COM88')

peripheralParams = namedtuple('peripheralParameters', 'peripheral, speed, port')
peripheralParamsDict = {
                            'uart' : peripheralParams('uart', '57600', targetUartPort),
                            'i2c'  : peripheralParams('i2c', '800',     buspalUartPort),
                            'spi'  : peripheralParams('spi', '600',     buspalUartPort),
                            # For can: '0' means 125k, '1' means 250k, '2' means 500k, '4' means 1M
                            'can'  : peripheralParams('can', '0',       buspalUartPort),
                            'usb'  : peripheralParams('usb', '',        '')
                       }
# set a default peripheral for unittest
# kDefaultPeripheral = 'can'
kDefaultPeripheral = os.environ.get('DEFAULT_PERIPHERAL', 'uart')


peripheral = os.environ.get('KIBBLE_PERIPHERAL', peripheralParamsDict[kDefaultPeripheral].peripheral)
speed = os.environ.get('KIBBLE_SPEED', peripheralParamsDict[kDefaultPeripheral].speed)
port = os.environ.get('KIBBLE_PORT', peripheralParamsDict[kDefaultPeripheral].port)
usePing = (os.environ.get('KIBBLE_USE_PING', 'True').lower() == 'true')
#bl_clockFlags = os.environ.get('CLOCK_FLAG', 0xFF)
#bl_clockDivider = os.environ.get("CLOCK_DIV", 0xFF)
bl_clockFlags = 0xFF
bl_clockDivider = 0xFF
 
loadTarget = False
resetTarget = False


# Specify the JLink USB ID that connets with the target.
# jlinkUsbId = os.environ.get('JLINK_USB_ID', '2104525845')
jlinkUsbId = os.environ.get('JLINK_USB_ID', '609302993')

deviceName = os.environ.get('DEVICE_NAME', devices.deviceDict[targetName.upper()])
##
# @brief Define the test vectors directory.
# @note The 'working' directory is one level above the test vectors directory.
vectorsDir = os.path.join(os.path.dirname(__file__), 'working', 'vectors')
