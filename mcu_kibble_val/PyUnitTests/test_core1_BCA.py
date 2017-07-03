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
from common import user_config_area

#############################################################################################################
#
# Test bootloader configuration area (BCA) under core 1.
#
#############################################################################################################
@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if(tgt.isDualCoreBootSupported == False):
        pytest.skip('%s does not support dual core boot!' %tgt.targetName)
                
class TestCore1BCATag:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.bootCore = 'core1'
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll = False)
              
        def teardown():
            pass
        request.addfinalizer(teardown)
       
    # Set the tag (tag != 'kcfg') in BCA, BCA data will never be used.  
    def test_core1_BCA_tag_not_kcfg(self, bl):
        user_config_area.BCA_tag_not_kcfg(bl, bootCore = self.bootCore)

#############################################################################################################
#
# The following cases are supported only when current peripheral is UART.
#
############################################################################################################# 
class TestCore1BCAWithUART:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        if(bl.peripheral != 'uart'):
            pytest.skip("This case is only supported on UART peripheral.")
            
        self.bootCore = 'core1'
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
    
    # Disable UART peripheral in BCA, it will not work fine. 
    def test_core1_BCA_disable_uart(self, bl):
        user_config_area.BCA_disable_peripheral(bl, bootCore = self.bootCore, peripheral = 'uart')
    
    # Enable UART peripheral while disable USB peripheral in BCA, UART should work fine.
    def test_core1_BCA_enable_uart_disable_usb(self, bl):
        user_config_area.BCA_disable_peripheral(bl, bootCore = self.bootCore, peripheral = 'usb')

#############################################################################################################
#
# The following cases are supported only when current peripheral is I2C.
#
############################################################################################################# 
class TestCore1BCAWithI2C:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        if(bl.peripheral != 'i2c'):
            pytest.skip("This case is only supported on I2C peripheral.")
            
        self.bootCore = 'core1'
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)  
    
    # Disable I2C peripheral in BCA, it will not work fine.      
    def test_core1_BCA_disable_i2c(self, bl):
        user_config_area.BCA_disable_peripheral(bl, bootCore = self.bootCore, peripheral = 'i2c')
     
    # Set i2c slave address in BCA, i2c will work fine with the setting i2c slave address.
    def test_core1_BCA_i2c_slave_address(self, bl):
        user_config_area.BCA_i2c_slave_address(bl, bootCore = self.bootCore)
        
#############################################################################################################
#
# The following cases are supported only when current peripheral is SPI.
#
############################################################################################################# 
class TestCore1BCAWithSPI:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        if(bl.peripheral != 'spi'):
            pytest.skip("This case is only supported on SPI peripheral.")
            
        self.bootCore = 'core1'
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown) 
    
    # Disable SPI peripheral in BCA, it will not work fine.            
    def test_core1_BCA_disable_spi(self, bl):
        user_config_area.BCA_disable_peripheral(bl, bootCore = self.bootCore, peripheral ='spi')

#############################################################################################################
#
# The following cases are supported only when current peripheral is FlexCAN.
#
############################################################################################################# 
class TestCore1BCAWithCAN:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        if(bl.peripheral != 'can'):
            pytest.skip("This case is only supported on CAN peripheral.")
            
        self.bootCore = 'core1'
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown) 
    
    # Disable FlexCAN peripheral in BCA, it will not work fine.         
    def test_core1_BCA_disable_can(self, bl):
        user_config_area.BCA_disable_peripheral(bl, bootCore = self.bootCore, peripheral = 'can')
    
    # Set FlexCAN txId and rxId in BCA, FlexCAN will work fine with the setting txId and rxId.
    def test_core1_BCA_can_slave_txrx_id(self, bl):
        user_config_area.BCA_can_slave_txrx_id(bl, bootCore = self.bootCore)
    
    # When canConfig1 = 0xF0, canConfig2 = 0xFFFF, FlexCAN only works on 125kbps
    def test_core1_BCA_set_canConfig1_0xF0(self, bl):
        user_config_area.BCA_set_canConfig1_canConfig2(bl, bootCore = self.bootCore, canConfig1 = 0xF0, canConfig2 = 0xFFFF)
            
    # When canConfig1 = 0xF1, canConfig2 = 0xFFFF, FlexCAN only works on 250kbps
    def test_core1_BCA_set_canConfig1_0xF1(self, bl):
        user_config_area.BCA_set_canConfig1_canConfig2(bl, bootCore = self.bootCore, canConfig1 = 0xF1, canConfig2 = 0xFFFF)
            
    # When canConfig1 = 0xF2, canConfig2 = 0xFFFF, FlexCAN only works on 500kbps
    def test_core1_BCA_set_canConfig1_0xF2(self, bl):
        user_config_area.BCA_set_canConfig1_canConfig2(bl, bootCore = self.bootCore, canConfig1 = 0xF2, canConfig2 = 0xFFFF)
            
    # When canConfig1 = 0xF4, canConfig2 = 0xFFFF, FlexCAN only works on 1Mbps
    def test_core1_BCA_set_canConfig1_0xF4(self, bl):
        user_config_area.BCA_set_canConfig1_canConfig2(bl, bootCore = self.bootCore, canConfig1 = 0xF4, canConfig2 = 0xFFFF)

#############################################################################################################
#
# The following cases are supported only when current peripheral is USB.
#
############################################################################################################# 
class TestCore1BCAWithUSB:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        if(bl.peripheral != 'usb'):
            pytest.skip("This case is only supported on USB peripheral.")
            
        self.bootCore = 'core1'
        common_util.setup_test_environment(bl, bootCore = self.bootCore, needFlashEraseAll = False)
             
        def teardown():
            pass
        request.addfinalizer(teardown)
                   
    # Disable USB peripheral in BCA, it will not work fine. 
    def test_core1_BCA_disable_usb(self, bl):
        user_config_area.BCA_disable_peripheral(bl, bootCore = self.bootCore, peripheral = 'usb')     

    # Set USB Vid and Pid in BCA, USB will work fine with the setting Vid and Pid.        
    def test_core1_BCA_usb_vid_pid(self, bl):
        user_config_area.BCA_usb_vid_pid(bl, bootCore = self.bootCore) 
        
     