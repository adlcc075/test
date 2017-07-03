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

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util
from common import security_boot



#############################################################################################################
#
# Test cases for security boot from core1:
#
###############################################################################################
## The certificate table can contain one or more certificates. Certificates must be positioned 
## in the table starting with the root certificate, followed by each subsequent certificate in 
## the chain in order of signing. The final certificate in the table is called the image signing 
## certificate. 
## Using a single certificate is allowed. In this case, the sole certificate must be self-signed 
## and must not be a CA. If multiple certificates are used, the root must be self-signed and 
## all but the last must be CAs.
## The two most common configurations will be:
## One self-signed certificate. 
## Self-signed root CA certificate, followed by image signing certificate which is itself signed 
## by the root certificate. 
###############################################################################################
##############################################################################################################
@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if(tgt.isDualCoreBootSupported == False):
        pytest.skip('%s does not support dual core boot!' %tgt.targetName)
     
class TestCore1SecureBoot:
    @pytest.fixture(autouse = True)
    def setup(self, request, bl):
        self.core = 'core1'
        self.originalImage = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\aut_image_gen\\led_demo_core1_0000.bin'))
        common_util.setup_test_environment(bl, bootCore = self.core, needFlashEraseAll = False)   
        def teardown():
            pass
        request.addfinalizer(teardown)

    ####################################################################################################
    # Using a single certificate, that is, the image signing certificate is the self signed certificate.  
    # Case 1: enable secure boot, enable secure boot developement mode
    # Case 2: enable secure boot, disable secure boot developement mode
    # Case 3: disable secure boot, enable secure boot developement mode
    # Case 4: disable secure boot, disable secure boot developement mode,
    # all with the image signing certificate no CA.
    ####################################################################################################
    def test_core1_enableSecureBoot_enbaleDevMode_singleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 1,         # One self-signed certificate
                                    isImageSigningCertCA = False,# The signing certificate is not CA.  
                                    secureBootEnable = True,     # Enable secure boot   
                                    devModeEnable = True,        # Enable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )
    def test_core1_enableSecureBoot_disaleDevMode_singleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 1,         # One self-signed certificate
                                    isImageSigningCertCA = False,# The signing certificate is not CA.  
                                    secureBootEnable = True,     # Enable secure boot   
                                    devModeEnable = False,       # Disable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )
    def test_core1_disableSecureBoot_enbaleDevMode_singleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 1,         # One self-signed certificate
                                    isImageSigningCertCA = False,# The signing certificate is not CA.  
                                    secureBootEnable = False,    # Disable secure boot   
                                    devModeEnable = True,        # Enable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )
    def test_core1_disableSecureBoot_disaleDevMode_singleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 1,         # One self-signed certificate
                                    isImageSigningCertCA = False,# The signing certificate is not CA.  
                                    secureBootEnable = False,     # Disable secure boot   
                                    devModeEnable = False,        # Disable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )


    ####################################################################################################
    # Using multiple certificates, that is, Self-signed root CA certificate, followed by 
    # image signing certificate which is signed by root certificate.  
    # Case 1: enable secure boot, enable secure boot developement mode
    # Case 2: enable secure boot, disable secure boot developement mode
    # Case 3: disable secure boot, enable secure boot developement mode
    # Case 4: disable secure boot, disable secure boot developement mode,
    # all with the image signing certificate no CA.
    ####################################################################################################
    def test_core1_enableSecureBoot_enbaleDevMode_multipleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 2,         # Self-signed root CA certificate, followed by image signing certificate which is signed by root certificate.
                                    isImageSigningCertCA = False,# The signing certificate is not CA.  
                                    secureBootEnable = True,     # Enable secure boot   
                                    devModeEnable = True,        # Enable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )
    def test_core1_enableSecureBoot_disaleDevMode_multipleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 2,         # Self-signed root CA certificate, followed by image signing certificate which is signed by root certificate.
                                    isImageSigningCertCA = False,# The signing certificate is not CA.  
                                    secureBootEnable = True,     # Enable secure boot   
                                    devModeEnable = False,       # Disable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )
    def test_core1_disableSecureBoot_enbaleDevMode_multipleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 2,         # Self-signed root CA certificate, followed by image signing certificate which is signed by root certificate.
                                    isImageSigningCertCA = False,# The signing certificate is not CA.  
                                    secureBootEnable = False,    # Disable secure boot   
                                    devModeEnable = True,        # Enable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )
    def test_core1_disableSecureBoot_disaleDevMode_multipleCert_ImageSignCertIsNotCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 2,          # Self-signed root CA certificate, followed by image signing certificate which is signed by root certificate.
                                    isImageSigningCertCA = False, # The signing certificate is not CA.  
                                    secureBootEnable = False,     # Disable secure boot   
                                    devModeEnable = False,        # Disable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )

        
    ##############################################################################################
    # For K3S, there are four root certificates (the SHA256 hash value) are resident in IFR 0x9C, 
    # 0x9D, 0x9E, 0x9F, respectively. Each root certificate can be revoked by programming 0s to its
    # corresponding IFR. If the root certificate is revoked and the image signing certificate is
    # derived from this root certificate, authetication check will fail, and the image can not be
    # booted successfully.
    ##############################################################################################         
    def test_core1_secure_boot_revoke_root_cert_table_entry0(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 1,        
                                    isImageSigningCertCA = False,   # The signing certificate is not CA.
                                    secureBootEnable = True,        # Enable secure boot   
                                    devModeEnable = False,          # Disable secure boot developement mode         
                                    rootCertTableEntryIndex = 0     # rootCertTableEntryIndex = 0
                 )  
    def test_core1_secure_boot_revoke_root_cert_table_entry1(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 2,        
                                    isImageSigningCertCA = False,   # The signing certificate is not CA.
                                    secureBootEnable = True,        # Enable secure boot   
                                    devModeEnable = False,          # Disable secure boot developement mode         
                                    rootCertTableEntryIndex = 1     # rootCertTableEntryIndex = 1
                 )  
    def test_core1_secure_boot_revoke_root_cert_table_entry2(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 3,        
                                    isImageSigningCertCA = False,   # The signing certificate is not CA.
                                    secureBootEnable = True,        # Enable secure boot   
                                    devModeEnable = False,          # Disable secure boot developement mode         
                                    rootCertTableEntryIndex = 2     # rootCertTableEntryIndex = 2
                 ) 
    def test_core1_secure_boot_revoke_root_cert_table_entry3(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 4,        
                                    isImageSigningCertCA = False,   # The signing certificate is not CA.
                                    secureBootEnable = True,        # Enable secure boot   
                                    devModeEnable = False,          # Disable secure boot developement mode         
                                    rootCertTableEntryIndex = 3     # rootCertTableEntryIndex = 3
                 )   


    ####################################################################################################
    # Using a single certificate, that is, the image signing certificate is the self signed certificate. 
    # Case 1: enable secure boot, disable secure boot developement mode with image signing certificate CA.
    ####################################################################################################       
    def test_core1_enableSecureBoot_enbaleDevMode_singleCert_ImageSignCertIsCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 1,         # One self-signed certificate.
                                    isImageSigningCertCA = False,# The signing certificate is CA.  
                                    secureBootEnable = True,     # Enable secure boot   
                                    devModeEnable = False,       # Disable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )


    ####################################################################################################
    # Using multiple certificates, that is, Self-signed root CA certificate, followed by 
    # image signing certificate which is signed by root certificate.  
    # Case 1: enable secure boot, disable secure boot developement mode with image signing certificate CA.
    ####################################################################################################   
    def test_core1_enableSecureBoot_enbaleDevMode_multipleCert_ImageSignCertIsCA(self, bl):
        security_boot.security_boot(bl, self.core,
                                    self.originalImage,         
                                    certChainLength = 2,         # Self-signed root CA certificate, followed by image signing certificate which is signed by root certificate.
                                    isImageSigningCertCA = False,# The signing certificate is CA.  
                                    secureBootEnable = True,     # Enable secure boot   
                                    devModeEnable = False,       # Disable secure boot developement mode   
                                    rootCertTableEntryIndex = -1
                 )

 
