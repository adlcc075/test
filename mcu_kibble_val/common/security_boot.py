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
import glob
import subprocess
import random
import string
 
filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env
 
from fsl import bootloader
from common import common_util
# The length of the chain certificates in order of signing
def generate_certificates(certChainLength, rsa_key_length = 4096):
    assert(certChainLength > 0)
    v3_noca_ext = os.path.abspath(os.path.join(filePath, 'working\\vectors\\security_boot\\aut_image_gen\\cfg\\v3_noca.ext'))
    v3_ca_ext = os.path.abspath(os.path.join(filePath, 'working\\vectors\\security_boot\\aut_image_gen\\cfg\\v3_ca.ext'))
    folder = os.path.abspath(os.path.join(filePath, 'working\\vectors\\security_boot\\certs\\certs_chain_len_%d'%certChainLength))
    if not os.path.exists(folder):
            os.makedirs(folder)
    os.chdir(folder)
 
    for format in ['*.pem', '*.csr', '*.crt']: 
        for tempFile in glob.glob(format):
            try:
                os.remove(tempFile)
            except IOError as e:
                print e
            except WindowsError as e:
                print e
     
    for i in xrange(certChainLength):
        # Generate private key
        cmd = "openssl genrsa -out ca%d_privatekey_rsa.pem %d"%(i, rsa_key_length)
        print cmd
        os.system(cmd)
        # Generate Certificate Signing Request (CSR)
        cmd = "openssl req -new -key ca%d_privatekey_rsa.pem -out ca%d_v3.csr -extensions v3_ca"%(i, i)
        p = subprocess.Popen(cmd, stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell = False) 
        CommonName = ''.join(random.sample(string.ascii_letters, 10))
        print p.communicate(input="CN\nChina\nsuzhou\nNXP\nMCUSW\n%s\nfangqiu.wang@nxp.com\npassword\nnxp\n"%CommonName)[0]
        # Generate singed certificate
        if i == 0:
            # Generate self-signed root certificate, CA
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -signkey ca%d_privatekey_rsa.pem -sha256 -outform der -out signed_cert%d_ca.der.crt -extfile "%s"' %(i, i, i, v3_ca_ext)
            print cmd
            os.system(cmd)
 
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -signkey ca%d_privatekey_rsa.pem -sha256 -out signed_cert%d_ca.pem.crt -extfile "%s"' %(i, i, i, v3_ca_ext)
            print cmd
            os.system(cmd)
 
            # Generate self-signed root certificate, no CA
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -signkey ca%d_privatekey_rsa.pem -sha256 -outform der -out signed_cert%d_noca.der.crt -extfile "%s"' %(i, i, i, v3_noca_ext)
            print cmd
            os.system(cmd)
 
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -signkey ca%d_privatekey_rsa.pem -sha256 -out signed_cert%d_noca.pem.crt -extfile "%s"' %(i, i, i, v3_noca_ext)
            print cmd
            os.system(cmd)   
        else:
            # Generate signed certificate, CA
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -CA signed_cert%d_ca.pem.crt -CAkey ca%d_privatekey_rsa.pem -set_serial 01 -sha256 -outform der -out signed_cert%d_ca.der.crt -extfile "%s"' %(i, i-1, i-1, i, v3_ca_ext)
            print cmd
            os.system(cmd)
             
 
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -CA signed_cert%d_ca.pem.crt -CAkey ca%d_privatekey_rsa.pem -set_serial 02 -sha256 -out signed_cert%d_ca.pem.crt -extfile "%s"' %(i, i-1, i-1, i, v3_ca_ext)
            print cmd
            os.system(cmd)
 
            # Generate signed certificate, no CA
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -CA signed_cert%d_ca.pem.crt -CAkey ca%d_privatekey_rsa.pem -set_serial 03 -sha256 -outform der -out signed_cert%d_noca.der.crt -extfile "%s"' %(i, i-1, i-1, i, v3_noca_ext)
            print cmd
            os.system(cmd)
 
            cmd = 'openssl x509 -req -days 365 -in ca%d_v3.csr -CA signed_cert%d_ca.pem.crt -CAkey ca%d_privatekey_rsa.pem -set_serial 04 -sha256 -out signed_cert%d_noca.pem.crt -extfile "%s"' %(i, i-1, i-1, i, v3_noca_ext)
            print cmd
            os.system(cmd)
# generate_certificates(certChainLength = 1, rsa_key_length = 4096)
# generate_certificates(certChainLength = 2, rsa_key_length = 4096)
# generate_certificates(certChainLength = 3, rsa_key_length = 2048)
# generate_certificates(certChainLength = 4, rsa_key_length = 2048)
#################################################################################################
# # @breif Use aut_image_gen.exe tool to generate signed image.
#  @parameter    bl                    singleton instance generated in conftest.py
#                originalImage         original boot image
#                isImageSigningCertCA  True   ram start address
#                memIndex  0, 1, 2, ... flash/ram index
def generate_signed_image(bl, originalImage, isImageSigningCertCA, usedRootCertIndex):
    # Four independent root certificates
    rootCert0 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_1\\signed_cert0_ca.der.crt'))
    rootCert1 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_2\\signed_cert0_ca.der.crt'))
    rootCert2 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_3\\signed_cert0_ca.der.crt'))
    rootCert3 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_4\\signed_cert0_ca.der.crt'))
         
    if(usedRootCertIndex == 0):
        # The cert chain length is 1, that is there is only one root certificate, and it is self-signed.
        # and this root certificate is also the image signing certificate.
        certCount = 0        
        imageSigningCertPrivateKey = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_1\\ca0_privatekey_rsa.pem'))
        if(isImageSigningCertCA == False):
            rootCert0 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_1\\signed_cert0_noca.der.crt'))
        else:
            rootCert0 = rootCert0
        imageSigningCert = rootCert0
        certContent = ""
         
    elif(usedRootCertIndex == 1):
        # The cert chain length is 2, that is 1 self-signed root certificate, 
        # followed by the image signing certificate which is signed by the root certificate.
        certCount = 1
        imageSigningCertPrivateKey = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_2\\ca1_privatekey_rsa.pem'))
        if(isImageSigningCertCA == False):
            imageSigningCert = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_2\\signed_cert1_noca.der.crt'))
        else:
            imageSigningCert = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_2\\signed_cert1_ca.der.crt'))
         
        cert0 = imageSigningCert
         
        certContent = "Cert0=%s\n"%(cert0)
     
    elif(usedRootCertIndex == 2):
        # The cert chain length is 3, the root certificate is followed by 2 subsequent cetificates in 
        # the chain in order of signing, the last certificate is image signing certificate.
        certCount = 2
        imageSigningCertPrivateKey = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_3\\ca2_privatekey_rsa.pem'))
        if(isImageSigningCertCA == False):
            imageSigningCert = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_3\\signed_cert2_noca.der.crt'))
        else:
            imageSigningCert = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_3\\signed_cert2_ca.der.crt'))
         
        cert1 = imageSigningCert
        cert0 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_3\\signed_cert1_ca.der.crt'))
       
        certContent = "Cert0=%s\n"%(cert0) + "Cert1=%s\n"%(cert1)
          
    elif(usedRootCertIndex == 3):
        # The cert chain length is 4, the root certificate is followed by 3 subsequent cetificates in 
        # the chain in order of signing, the last certificate is image signing certificate.
        certCount = 3
        imageSigningCertPrivateKey = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_4\\ca3_privatekey_rsa.pem'))
        if(isImageSigningCertCA == False):
            imageSigningCert = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_4\\signed_cert3_noca.der.crt'))
        else:
            imageSigningCert = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_4\\signed_cert3_ca.der.crt'))
         
        cert2 = imageSigningCert
        cert1 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_4\\signed_cert2_ca.der.crt'))
        cert0 = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\certs\\certs_chain_len_4\\signed_cert1_ca.der.crt'))
     
        certContent = "Cert0=%s\n"%(cert0) + "Cert1=%s\n"%(cert1) + "Cert2=%s\n"%(cert2)
     
    assert((usedRootCertIndex >= 0) and (usedRootCertIndex <= 3))
    certSignConfig = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\aut_image_gen\\cfg\\lastCA_%s_rootCertIndex_%d.cfg'%(isImageSigningCertCA,usedRootCertIndex)))
    fObj = file(certSignConfig, 'w+')
    fObj.write("[PrivateKey]\n")
    fObj.write("PrivateKey=%s\n"%imageSigningCertPrivateKey)
    fObj.write("[RootCertList]\n")
    fObj.write("RootCertCount=4\n")
    fObj.write("RootCertUseAsRoot=%d\n"%usedRootCertIndex)
    fObj.write("RootCert0=%s\n"%rootCert0)
    fObj.write("RootCert1=%s\n"%rootCert1)
    fObj.write("RootCert2=%s\n"%rootCert2)
    fObj.write("RootCert3=%s\n"%rootCert3)
    fObj.write("[CertList]\n")
    fObj.write("CertCount=%d\n"%certCount)    
    fObj.write(certContent)
    fObj.close()
    # Genaerate signed image
    aut_image_gen = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\aut_image_gen\\aut_image_gen.exe'))
    signedImage = os.path.abspath(os.path.join(bl.vectorsDir, 'security_boot\\aut_image_gen\\led_demo_signed.bin'))
    cmd = aut_image_gen + " %s %s %s" %(originalImage, certSignConfig, signedImage)
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = p.communicate()[0]
    # Get the SHA-256 hash of root public key hash value, which should be programmed to IFR 0x94~0x97.
    output = output.split('\n')
    for line in output:
        if 'kht SHA256 HASH:' in line:
            rkth = line.replace('kht SHA256 HASH:', '')
            rkth0 = rkth[1:17]
            rkth1 = rkth[17:33]
            rkth2 = rkth[33:49]
            rkth3 = rkth[49:65]
 
    return (signedImage, rkth0, rkth1, rkth2, rkth3)

def security_boot(bl, bootCore,                 # Which core does ROM code boot from?
                      originalImage,            # Original led demo for boot core, which is not signed
                      certChainLength,          # The length of the chain certificates in order of signing
                      isImageSigningCertCA,     # Is the image signing certificate CA? 
                      secureBootEnable,         # Secure boot flag
                      devModeEnable,            # Secure boot development mode flag
                      rootCertTableEntryIndex   # Specify which root certificate table entry will be revoked,
                                                # should be 0, 1, 2, 3, other values means no entry will be revoked.
                 ):
    # Get flash start address
    if(bootCore == 'core0'):
        flashStartAddress = common_util.get_memory_start_address(bl, 'flash', 0)
    elif(bootCore == 'core1'):
        flashStartAddress = common_util.get_memory_start_address(bl, 'flash', 1)
        
    (signedImage, rkth0, rkth1, rkth2, rkth3) = generate_signed_image(bl, originalImage, isImageSigningCertCA, usedRootCertIndex = certChainLength - 1)
    # Write the signed led demo to flash
    status, results = bl.write_memory(flashStartAddress, signedImage)
    assert status == bootloader.status.kStatus_Success
    # Program the RKTH to IFR 0x94~0x97
    status, results = bl.flash_program_once(0x94, 8, rkth0, 'msb')
    assert status == bootloader.status.kStatus_Success
    status, results = bl.flash_program_once(0x95, 8, rkth1, 'msb')
    assert status == bootloader.status.kStatus_Success
    status, results = bl.flash_program_once(0x96, 8, rkth2, 'msb')
    assert status == bootloader.status.kStatus_Success
    status, results = bl.flash_program_once(0x97, 8, rkth3, 'msb')
    assert status == bootloader.status.kStatus_Success
    #Enable/Disable secure boot and secure boot development mode
    if(secureBootEnable == True and devModeEnable == True):
        status, results = bl.flash_program_once(0x98, 8, 'fffffffffffffffa', 'lsb')
        assert status == bootloader.status.kStatus_Success
    elif(secureBootEnable == True and devModeEnable == False):
        status, results = bl.flash_program_once(0x98, 8, 'fffffffffffffff1', 'lsb')
        assert status == bootloader.status.kStatus_Success
    elif(secureBootEnable == False and devModeEnable == True):
        status, results = bl.flash_program_once(0x98, 8, 'fffffffffffffff7', 'lsb')
        assert status == bootloader.status.kStatus_Success
    elif(secureBootEnable == False and devModeEnable == False):
        status, results = bl.flash_program_once(0x98, 8, 'fffffffffffffffc', 'lsb')
        assert status == bootloader.status.kStatus_Success
 
    # Revoke the root certificate table entry, the entry number should be 0~3, 
    # other values means no entry will be revoked.
    if(rootCertTableEntryIndex < 0 or rootCertTableEntryIndex > 3):
        needRevoke = False
    else:
        status, results = bl.flash_program_once(0x9c + rootCertTableEntryIndex, 8, '0000000000000000', 'lsb')
        assert status == bootloader.status.kStatus_Success
        needRevoke = True
         
    # Reset target and wait about 5s
    status, results = bl.reset()
    assert status == bootloader.status.kStatus_Success
    time.sleep(50)
    status, results = bl.get_property(1)
    # The led demo will never run in the following two cases:
    # 1. when secure boot is enabled and the image signing certificate is CA,
    # 2. when secure boot is enabled and the the root certificate table entry is revoked.
    if((secureBootEnable == True) and (isImageSigningCertCA == True or needRevoke == True)):
        assert status == bootloader.status.kStatus_Success
    else:
        assert status != bootloader.status.kStatus_Success