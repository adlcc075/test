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

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util
from PyUnitTests import bltest_config

supportedPeripheralSpeed_spi = [5, 10, 20, 50, 100, 200, 500]

@pytest.mark.skipif(bltest_config.peripheral != 'spi', reason = 'Test class only valid for spi peripheral.')
class TestSupportedSpiClock:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        pass
        def teardown():
            pass
        request.addfinalizer(teardown)
    
    
    @pytest.mark.parametrize('spiSpeed', supportedPeripheralSpeed_spi)
    def test_all_supported_spi_speed(self, bl, spiSpeed):
        # Change the spi speed
        bl.set_SPI_speed(spiSpeed)
        common_util.reset_with_check(bl)
        # 1. Get available flash region
        availableRegionStartAddress, availableRegionEndAddress, availableRegionSize = common_util.get_available_memory_region(bl, 'flash')
         
        # 2. Erase all the available flash region
        status, results = bl.flash_erase_region(availableRegionStartAddress, availableRegionSize)
        assert status == bootloader.status.kStatus_Success
         
        # 3. Program all the available flash region
        randomFile = common_util.generate_random_data_file(bl, availableRegionStartAddress, availableRegionSize)
        status, results = bl.write_memory(availableRegionStartAddress, randomFile)
        assert status == bootloader.status.kStatus_Success
         
        # 4. Read back the data
        readFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(availableRegionStartAddress, availableRegionSize, readFile)
        assert status == bootloader.status.kStatus_Success
         
        # 5. Verify data
        randomFileObj = file(randomFile, 'rb')
        data1 = randomFileObj.read()
        readFileObj = file(readFile, 'rb')
        data2 = readFileObj.read()
        assert data1 == data2