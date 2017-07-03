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
import string
import time
import subprocess

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util       

class SBFile:
    # # @breif Initialize the bd file content. 
    #          Generally, bd file content is composed by 4 parts, e.g. options, sources, keyblob, section.
    @staticmethod
    def bd_content_init(bl, needSign=False):
        options = ''
        if(bl.target.isSB2Supported):
            if(needSign):
                flags = 0x08
            else:
                flags = 0x00
            options += 'options{\n'
            options += '    flags = 0x%x;\n'%flags
            options += '    buildNumber = 0xab;\n' # In SB2 this label is used for build number
            options += '    productVersion = "1.00.00";\n'
            options += '    componentVersion = "1.00.00";\n'
            options += '}\n'
        else:
            options += 'options{\n}\n'
        
        sources     = 'sources{\n}\n'
        keyblob     = 'keyblob(0){\n}\n'
        section     = 'section(0){\n}\n'
        return(options, sources, keyblob, section)

    # # @breif  Update the sources field of the bd file content.
    @staticmethod
    def bd_content_update_for_sources(sources, fileName, filePath):
        addedInfo = '    %s = "%s";\n' %(fileName, filePath)
        sources = sources[:-2] + addedInfo + sources[-2:]
        return sources
    
    # # @breif  Update the keyblob field of the bd file content.
    @staticmethod
    def bd_content_update_for_keyblob(keyblob, startAddress, endAddress, key, counter):
        addedInfo  = '    (\n'
        addedInfo += '        start = 0x%x,\n' %startAddress
        addedInfo += '        end = 0x%x,\n' %endAddress
        addedInfo += '        key = "%s",\n' %key
        addedInfo += '        counter = "%s"\n' %counter
        addedInfo += '    )\n'
        keyblob = keyblob[:-2] + addedInfo + keyblob[-2:]
        return keyblob  
    
    # @breif Update the section field of the bd file content when adding fill-memory sb command.
    @staticmethod
    def bd_content_update_for_fill_memory(bdContent, pattern, format, fillStartAddress, fillEndAddress):
        (options, sources, keyblob, section) = (bdContent[0], bdContent[1], bdContent[2], bdContent[3])
        if format == 'byte':
           sectionAddedInfo = '    load 0x%x.b > 0x%x..0x%x;\n' %(pattern, fillStartAddress, fillEndAddress)
        elif format == 'short':
           sectionAddedInfo = '    load 0x%x.h > 0x%x..0x%x;\n' %(pattern, fillStartAddress, fillEndAddress)
        elif format == 'word':
           sectionAddedInfo = '    load 0x%x.w > 0x%x..0x%x;\n' %(pattern, fillStartAddress, fillEndAddress)
        
        section = section[:-2] + sectionAddedInfo + section[-2:]
        return (options, sources, keyblob, section)
    
    # @breif Update the section field of the bd file content when adding write-memory sb command.
    @staticmethod
    def bd_content_update_for_write_memory(bdContent, binFilePath, writeStartAddress, writeEndAddress):
        (options, sources, keyblob, section) = (bdContent[0], bdContent[1], bdContent[2], bdContent[3])
        fileName = "binFile" + ''.join(random.sample(string.digits, 5)) 
        sources = SBFile.bd_content_update_for_sources(sources, fileName, binFilePath)
        sectionAddedInfo = "    load %s > 0x%x..0x%x;\n" %(fileName, writeStartAddress, writeEndAddress)
        section = section[:-2] + sectionAddedInfo + section[-2:]
        return (options, sources, keyblob, section)
    
    # @breif Update the section field of the bd file content when adding flash-erase-region sb command.
    @staticmethod
    def bd_content_update_for_flash_erase_region(bdContent, eraseStartAddress, eraseEndAddress):    
        (options, sources, keyblob, section) = (bdContent[0], bdContent[1], bdContent[2], bdContent[3])
        sectionAddedInfo = "    erase 0x%x..0x%x;\n" %(eraseStartAddress, eraseEndAddress)
        section = section[:-2] + sectionAddedInfo + section[-2:]
        return (options, sources, keyblob, section)   
    
    # @breif Update the section field of the bd file content when adding flash-erase-all sb command.
    @staticmethod
    def bd_content_update_for_flash_erase_all(bdContent, flashType):    
        (options, sources, keyblob, section) = (bdContent[0], bdContent[1], bdContent[2], bdContent[3])
        if(flashType == 'internalFlash'):
            sectionAddedInfo = "    erase all;\n"
        elif(flashType == 'qspiFlash'):
            sectionAddedInfo = "    erase qspi all;\n"
        section = section[:-2] + sectionAddedInfo + section[-2:]
        return (options, sources, keyblob, section)
    
    # @breif Update the section field of the bd file content when adding flash-erase-all-unsecure sb command.
    @staticmethod
    def bd_content_update_for_flash_erase_all_unsecure(bdContent):    
        (options, sources, keyblob, section) = (bdContent[0], bdContent[1], bdContent[2], bdContent[3])
        sectionAddedInfo = "    erase unsecure all;\n"
        section = section[:-2] + sectionAddedInfo + section[-2:]
        return (options, sources, keyblob, section)
    
    # @breif Update the section field of the bd file content when configuring qspi with sb command.
    @staticmethod
    def bd_content_update_for_config_qspi(bdContent, qspiConfigBlock, qcbLocation):
        (options, sources, keyblob, section) = (bdContent[0], bdContent[1], bdContent[2], bdContent[3])
        fileName = "qspi_config_block"
        sources = SBFile.bd_content_update_for_sources(sources, fileName, qspiConfigBlock)
        sectionAddedInfo  = "    load %s > 0x%x;\n" %(fileName, qcbLocation)
        sectionAddedInfo += '    enable qspi 0x%x;\n' %(qcbLocation)
        section = section[:-2] + sectionAddedInfo + section[-2:]
        return (options, sources, keyblob, section)
    
    # @breif Generate the bd file according to the bd content.
    @staticmethod
    def bd_content_finish(bl, bdContent):
        print("%s%s%s" %("-"*20, "BD file contents start", "-"*20))
        print("%s%s%s%s" %(bdContent[0], bdContent[1], bdContent[2], bdContent[3]))
        print("%s%s%s" %("-"*20, "BD file contents end", "-"*20))
        bdFile = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', '%s.bd'%bl.target.targetName))
        fileObj = file(bdFile,'w+')
        fileObj.write(bdContent[0] + bdContent[1] + bdContent[2] + bdContent[3])
        fileObj.close()
        return bdFile
    
    # # @breif Generate the corresponding sb file by elftosb tool.
    #  @parameters    
    #   bl               Singleton instance generated in conftest.py
    #   bdFile           Binary Description File that contains a series of sb commands.
    #   needEncrypt      True means this sb file needs to be encrypted, while False means no need to be encrypted.
    #                         Note: For K3S, sb file is forced to be encrypted.
    #   encryptionKey    For K3S sb file is encrypted by AES-256, so the key is 32 Bytes,
    #                    before the K3S, it is encrypted by AES-128, so the key is 16 Bytes.
    #
    #   needSign         True mean this sb file will be signed, while False means it will not be signed.
    #   s                A list that contains the private keys for signing.
    #   S                A list that contains certificate files for signing.
    #   R                A list that contains the root certificates for verifying other certificates.                           
    @staticmethod
    def generate_sb_file(bl, bdFile, needEncrypt, encryptionKey, needSign, s=[], S=[], R=[]):        
        keyFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', 'key.txt'))
        elftosb = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', 'elftosb.exe'))
        
        if(bl.target.isSB2Supported):
            sbFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', '%s.sb2'%bl.target.targetName))
            if(needEncrypt):
                # write the given key to the key.txt file
                fileObj = file(keyFilePath, 'w+')
                fileObj.write(encryptionKey)
                fileObj.close()
                print('Generating encrypted(key = "%s")' %encryptionKey),
                elftosbCommandLine = "%s -V -d -f kinetisk3 -k %s -c %s -o %s" %(elftosb, keyFilePath, bdFile, sbFilePath)
            else:
                print('Generating all 1s encrypted'),
                elftosbCommandLine = "%s -V -d -f kinetisk3 -z -c %s -o %s" %(elftosb, bdFile, sbFilePath)
            
            if(needSign):
                print("and signed SB2 file:")
                for i in xrange(len(s)):
                    elftosbCommandLine += " -s %s -S %s -R %s " %(s[i], S[i], R[i])
            else:
                print("SB2 file:")
                pass
        else:
            sbFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', '%s.sb'%bl.target.targetName))
            if(needEncrypt):
                # write the given key to the key.txt file
                fileObj = file(keyFilePath, 'w+')
                fileObj.write(encryptionKey)
                fileObj.close()
                print('Generating encrypted (key = "%s") SB file:' %encryptionKey)
                elftosbCommandLine = "%s -V -d -k %s -c %s -o %s" %(elftosb, keyFilePath, bdFile, sbFilePath)
            else:
                print('Generating unencrypted SB file:')
                elftosbCommandLine = "%s -V -d -c %s -o %s" %(elftosb, bdFile, sbFilePath)
        
        
        print elftosbCommandLine
        p = subprocess.Popen(elftosbCommandLine, stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        output = p.communicate()[0]
        print output
        print('-'*60 + ' SB file generated successfully.')
        return sbFilePath
