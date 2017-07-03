#! /usr/bin/env python

# Copyright (c) 2015 Freescale Semiconductor, Inc.
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
import random
import pytest
import subprocess
from collections import namedtuple

sbCommand_writeMemory           = namedtuple('sbComamnd', 'cumulativeWrite, length, data, dataType, startAddress, endAddress')
sbCommand_fillMemory            = namedtuple('sbCommand', 'cumulativeFill, pattern, patternFormat, startAddress, endAddress')
sbCommand_flashEraseRegion      = namedtuple('sbCommand', 'startAddress, endAddress')
sbCommand_flashEraseAllUnsecure = namedtuple('sbCommand', '')
sbCommand_internalFlashEraseAll = namedtuple('sbCommand', '')
sbCommand_qspiFlashEraseAll     = namedtuple('sbCommand', '')
sbCommand_enableQspi            = namedtuple('sbCommand', 'qspiConfigBlock, qcbLocation')
sbCommand_jumpEntryPoint        = namedtuple('sbCommand', '')
sbCommand_jumpStackPoint        = namedtuple('sbCommand', 'hasArg, arg')
sbCommand_programIfr            = namedtuple('sbCommand', 'ifrIndex1, ifrValue1, ifrIndex2, ifrValue2, ifrIndex3, ifrValue3, ifrIndex4, ifrValue4')
sbCommand_call                  = namedtuple('sbCommand', 'hasArg, arg')
sbCommand_reset                 = namedtuple('sbCommand', '')
sbCommand_otfadDecryption       = namedtuple('sbCommand',
                                'qspiDemo, \
                                 OTFAD_CTX0_START_ADDR, OTFAD_CTX0_END_ADDR, OTFAD_CTX0_KEY, OTFAD_CTX0_CTR, \
                                 OTFAD_CTX1_START_ADDR, OTFAD_CTX1_END_ADDR, OTFAD_CTX1_KEY, OTFAD_CTX1_CTR, \
                                 OTFAD_CTX2_START_ADDR, OTFAD_CTX2_END_ADDR, OTFAD_CTX2_KEY, OTFAD_CTX2_CTR, \
                                 OTFAD_CTX3_START_ADDR, OTFAD_CTX3_END_ADDR, OTFAD_CTX3_KEY, OTFAD_CTX3_CTR, \
                                 encrypted_type_0, encrypted_data_0, data_location_start_0, data_location_end_0, \
                                 encrypted_type_1, encrypted_data_1, data_location_start_1, data_location_end_1, \
                                 encrypted_type_2, encrypted_data_2, data_location_start_2, data_location_end_2, \
                                 encrypted_type_3, encrypted_data_3, data_location_start_3, data_location_end_3, \
                                 Key_Encryption_Key, kek_location')

# ==================================================================================================================
# @breif Construct a dictionary for sb commands.
sbCmdDict = {
                'writeMemory'           : sbCommand_writeMemory,
                'fillMemory'            : sbCommand_fillMemory,
                'flashEraseRegion'      : sbCommand_flashEraseRegion,
                'flashEraseAllUnsecure' : sbCommand_flashEraseAllUnsecure,
                'internalFlashEraseAll' : sbCommand_internalFlashEraseAll,
                'qspiFlashEraseAll'     : sbCommand_qspiFlashEraseAll,
                'enableQspi'            : sbCommand_enableQspi,
                'jumpEntryPoint'        : sbCommand_jumpEntryPoint,
                'jumpStackPoint'        : sbCommand_jumpStackPoint,
                'programIfr'            : sbCommand_programIfr,
                'call'                  : sbCommand_call,
                'reset'                 : sbCommand_reset,
                'otfadDecryption'       : sbCommand_otfadDecryption
            }


# ==================================================================================================================
import common_util
# A simple string
kStringForSimpleSbFile = common_util.kStringForSimpleSbFile

# ==================================================================================================================

## @breif This function is to generate an sb file.
# @param: bl
#         encryptionType    'unencrypted'            generate sb file without encrytion
#                           'zeroKeyEncrypted'       generate sb file with zero key encryption
#                           'nonZeroKeyEncrypted'    generate sb file with non-zero key encryption
#         key               string type with length of 32, each character should be 0~F
#         *sbCommands       multipule input string parameters, should note the order of the parameter

def generate_sb_file(bl, encryptionType, key, *sbCommands):
    options     = 'options {\nflags=0x0;\n}\n'
    constants   = 'constants{\n}\n'
    sources     = 'sources{\n}\n'
    keyblob     = 'keyblob(0){\n}\n'
    section     = 'section(0){\n}\n'

    otfadContex0     = '\t' + '()' + '\n'
    otfadContex1     = '\t' + '()' + '\n'
    otfadContex2     = '\t' + '()' + '\n'
    otfadContex3     = '\t' + '()' + '\n'

    insertedOptions    = ''
    insertedConstants  = ''
    insertedSoreces    = ''
    insertedKeyblob    = ''
    insertedSection    = ''
    insertedContex0    = ''
    insertedContex1    = ''
    insertedContex2    = ''
    insertedContex3    = ''

    # Get the bd file contents according to the parameters.
    for i in range(0, len(sbCommands)):
        if sbCommands[i] == 'writeMemory':
            cumulativeWrite = sbCmdDict['writeMemory'].cumulativeWrite
            length = sbCmdDict['writeMemory'].length
            data = sbCmdDict['writeMemory'].data
            dataType = sbCmdDict['writeMemory'].dataType
            startAddress = sbCmdDict['writeMemory'].startAddress
            endAddress = sbCmdDict['writeMemory'].endAddress
            # -------------------------------------------------------------------------------------------------
            # write twice without erase.
            if cumulativeWrite:
                insertedSection += '    load "%s" > 0x%x..0x%x;\n' %(kStringForSimpleSbFile, startAddress, endAddress)
            else:
                pass
            # -------------------------------------------------------------------------------------------------
            if dataType == 'string':   # A simple string
                insertedSection += '    load "%s" > 0x%x..0x%x;\n' %(data, startAddress, endAddress)
            elif dataType == 'function':   # A simple function with string
                insertedSection += '    load %s > 0x%x..0x%x;\n' %(data, startAddress, endAddress)
                insertedSection += '    call 0x%x;\n' %(startAddress + 1)
            elif dataType == 'file_bin':  # common data with .bin format
                insertedSoreces += '    myBinFile = "%s";\n' %(data)
                insertedSection += '    load myBinFile > 0x%x..0x%x;\n' %(startAddress, endAddress)
            elif dataType == 'app_bin':    # application with .bin format
                insertedSoreces += '    appBinFile = "%s";\n' %(data)
                insertedSection += '    load appBinFile > 0x%x..0x%x;\n' %(startAddress, endAddress)
            elif dataType == 'app_out':    # application with .out format
                insertedSoreces += '    appOutFile = "%s";\n' %(data)
                insertedSection += "    load $A1*, $P1* from appOutFile;\n"
            elif dataType == 'app_srec':    # application with .srec format
                insertedSoreces += '    appSrecFile = "%s";\n' %(data)
                insertedSection += "    load appSrecFile;\n"
        elif sbCommands[i] == 'fillMemory':
            cumulativeFill = sbCmdDict['fillMemory'].cumulativeFill
            pattern = sbCmdDict['fillMemory'].pattern
            patternFormat = sbCmdDict['fillMemory'].patternFormat
            startAddress = sbCmdDict['fillMemory'].startAddress
            endAddress = sbCmdDict['fillMemory'].endAddress

            if patternFormat == 'byte':
                insertedSection += '    load 0x%x.b > 0x%x..0x%x;\n' %(pattern, startAddress, endAddress)
            elif patternFormat == 'short':
                insertedSection += '    load 0x%x.h > 0x%x..0x%x;\n' %(pattern, startAddress, endAddress)
            elif patternFormat == 'word':
                insertedSection += '    load 0x%x.w > 0x%x..0x%x;\n' %(pattern, startAddress, endAddress)

            # -------------------------------------------------------------------------------------------------
            # fill twice without erase.
            if cumulativeFill:
                insertedSection += '    load 0x%x.b > 0x%x..0x%x;\n' %(pattern + 1, startAddress, endAddress)
            else:
                pass
            # -------------------------------------------------------------------------------------------------
        elif sbCommands[i] == 'flashEraseRegion':
            startAddress = sbCmdDict['flashEraseRegion'].startAddress
            endAddress = sbCmdDict['flashEraseRegion'].endAddress
            insertedSection += '    erase 0x%x..0x%x;\n' %(startAddress, endAddress)
        elif sbCommands[i] == 'flashEraseAllUnsecure':
            insertedSection += '    erase unsecure all;\n'
        elif sbCommands[i] == 'internalFlashEraseAll':
            insertedSection += '    erase all;\n'
        elif sbCommands[i] == 'qspiFlashEraseAll':
            insertedSection += '    erase qspi all;\n'
        elif sbCommands[i] == 'enableQspi':
            qspiConfigBlock = sbCmdDict['enableQspi'].qspiConfigBlock
            qcbLocation = sbCmdDict['enableQspi'].qcbLocation
            insertedSoreces += '    qspiConfigBlock = "%s";\n' %(qspiConfigBlock)
            insertedSection += '    load qspiConfigBlock > 0x%x;\n' %(qcbLocation)
            insertedSection += '    enable qspi 0x%x;\n' %(qcbLocation)
        elif sbCommands[i] == 'jumpEntryPoint':
            if sbCmdDict['writeMemory'].dataType == 'app_bin':
                # if the app is the bin file, get the PC address in the bin file and jump to the PC address
                SP, PC = common_util.get_sp_pc_from_bin_file(sbCmdDict['writeMemory'].data)
                insertedSection += '    jump 0x%x;\n' %(PC)
            elif sbCmdDict['writeMemory'].dataType == 'app_out':
                insertedSection += '    jump appOutFile;\n'
            else:
                pass
        elif sbCommands[i] == 'jumpStackPoint':
            hasArg = sbCmdDict['jumpStackPoint'].hasArg
            arg = sbCmdDict['jumpStackPoint'].arg
            if sbCmdDict['writeMemory'].dataType == 'app_bin':
                # get the SP and PC address from the app bin file
                SP, PC = common_util.get_sp_pc_from_bin_file(sbCmdDict['writeMemory'].data)
                if not hasArg:
                    insertedSection += '    jump_sp 0x%x 0x%x;\n' %(SP, PC)
                else:
                    insertedSection += '    jump_sp 0x%x 0x%x(0x%x);\n' %(SP, PC, arg)
        elif sbCommands[i] == 'programIfr':
            ifrIndex1 = sbCmdDict['programIfr'].ifrIndex1
            ifrValue1 = sbCmdDict['programIfr'].ifrValue1
            ifrIndex2 = sbCmdDict['programIfr'].ifrIndex2
            ifrValue2 = sbCmdDict['programIfr'].ifrValue2
            ifrIndex3 = sbCmdDict['programIfr'].ifrIndex3
            ifrValue3 = sbCmdDict['programIfr'].ifrValue3
            ifrIndex4 = sbCmdDict['programIfr'].ifrIndex4
            ifrValue4 = sbCmdDict['programIfr'].ifrValue4

            insertedSection += '    load ifr 0x%x > 0x%x;\n' %(ifrValue1, ifrIndex1)
            insertedSection += '    load ifr 0x%x > 0x%x;\n' %(ifrValue2, ifrIndex2)
            insertedSection += '    load ifr 0x%x > 0x%x;\n' %(ifrValue3, ifrIndex3)
            insertedSection += '    load ifr 0x%x > 0x%x;\n' %(ifrValue4, ifrIndex4)
        elif sbCommands[i] == 'call':
            hasArg = sbCmdDict['call'].hasArg
            arg = sbCmdDict['call'].arg
            if hasArg:
                insertedSection += '    call appOutFile:program_SYST_RVR_register_with_arg' + '(' + ('0x%x' %arg) + ');\n'
            else:
                insertedSection += '    call appOutFile:program_SYST_RVR_register_without_arg' + '();\n'
        elif sbCommands[i] == 'reset':
            insertedSection += '    reset;\n'
        elif sbCommands[i] == 'otfadDecryption':
            #------------------------------------------------------------------------------
            # Get the values of OTFAD CONTEX0
            startAddress0 = sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR
            endAddress0   = sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR
            key0          = sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY
            counter0      = sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR
            #------------------------------------------------------------------------------
            # Get the values of OTFAD CONTEX1
            startAddress1 = sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR
            endAddress1   = sbCmdDict['otfadDecryption'].OTFAD_CTX1_END_ADDR
            key1          = sbCmdDict['otfadDecryption'].OTFAD_CTX1_KEY
            counter1      = sbCmdDict['otfadDecryption'].OTFAD_CTX1_CTR
            #------------------------------------------------------------------------------
            # Get the values of OTFAD CONTEX2
            startAddress2 = sbCmdDict['otfadDecryption'].OTFAD_CTX2_START_ADDR
            endAddress2   = sbCmdDict['otfadDecryption'].OTFAD_CTX2_END_ADDR
            key2          = sbCmdDict['otfadDecryption'].OTFAD_CTX2_KEY
            counter2      = sbCmdDict['otfadDecryption'].OTFAD_CTX2_CTR
            #------------------------------------------------------------------------------
            # Get the values of OTFAD CONTEX3
            startAddress3 = sbCmdDict['otfadDecryption'].OTFAD_CTX3_START_ADDR
            endAddress3   = sbCmdDict['otfadDecryption'].OTFAD_CTX3_END_ADDR
            key3          = sbCmdDict['otfadDecryption'].OTFAD_CTX3_KEY
            counter3      = sbCmdDict['otfadDecryption'].OTFAD_CTX3_CTR
            #=========================================================================================================
            # Generate bd contents for OTFAD CONTEX0 ~ CONTEX3
            #=========================================================================================================
            if (startAddress0 != None) and (endAddress0 != None) and (key0 != None) and (counter0 != None):
                insertedContex0 += '\n        start = 0x%x,' %(startAddress0)
                insertedContex0 += '\n        end = 0x%x,' %(endAddress0)
                insertedContex0 += '\n        key = "%s",' %(key0)
                insertedContex0 += '\n        counter = "%s"\n\t' %(counter0)
            else:
                pass    # If not have initial values, bypass this CONTEX.
            #------------------------------------------------------------------------------
            if (startAddress1 != None) and (endAddress1 != None) and (key1 != None) and (counter1 != None):
                insertedContex1 += '\n        start = 0x%x,' %(startAddress1)
                insertedContex1 += '\n        end = 0x%x,' %(endAddress1)
                insertedContex1 += '\n        key = "%s",' %(key1)
                insertedContex1 += '\n        counter = "%s"\n\t' %(counter1)
            else:
                pass    # If not have initial values, bypass this CONTEX.
            #------------------------------------------------------------------------------
            if (startAddress2 != None) and (endAddress2 != None) and (key2 != None) and (counter2 != None):
                insertedContex2 += '\n        start = 0x%x,' %(startAddress2)
                insertedContex2 += '\n        end = 0x%x,' %(endAddress2)
                insertedContex2 += '\n        key = "%s",' %(key2)
                insertedContex2 += '\n        counter = "%s"\n\t' %(counter2)
            else:
                pass
            #------------------------------------------------------------------------------
            if (startAddress3 != None) and (endAddress3 != None) and (key3 != None) and (counter3 != None):
                insertedContex3 += '\n        start = 0x%x,' %(startAddress3)
                insertedContex3 += '\n        end = 0x%x,' %(endAddress3)
                insertedContex3 += '\n        key = "%s",' %(key3)
                insertedContex3 += '\n        counter = "%s"\n\t' %(counter3)
            else:
                pass

            # In order to auto config the QSPI flash after reset, QCB should be loaded to 0x68000000.
            # Note that this operation must follow by 'enableQspi'
            insertedSection += '    load qspiConfigBlock > 0x%x;\n' %(0x68000000)
            #===================================================================
            # Generate bd contents for encrypt
            #===================================================================
            insertedSection += '\t' + 'encrypt(0){\n'
            #------------------------------------------------------------------------------
            # Get the encrypted data
            qspiDemo = sbCmdDict['otfadDecryption'].qspiDemo


            # Section data encrypted in the first keyblob region
            encrypted_type_0 = sbCmdDict['otfadDecryption'].encrypted_type_0
            encrypted_data_0 = sbCmdDict['otfadDecryption'].encrypted_data_0
            data_location_start_0  = sbCmdDict['otfadDecryption'].data_location_start_0
            data_location_end_0  = sbCmdDict['otfadDecryption'].data_location_end_0
            if encrypted_type_0 == 'encrypt_fill':
                insertedSection += '        load 0x%x.b > 0x%x..0x%x;\n' %(encrypted_data_0, data_location_start_0, data_location_end_0)
            elif encrypted_type_0 == 'encrypt_write':
                insertedSoreces += '    binFile0 = "%s";\n' %(encrypted_data_0)
                insertedSection += '        load binFile0 > 0x%x..0x%x;\n' %(data_location_start_0,data_location_end_0)
            elif encrypted_type_0 == 'encrypt_appSrecFile':
                insertedSoreces += '    appSrecFile = "%s";\n' %(qspiDemo)
                insertedSection += '        load appSrecFile;\n'
            elif encrypted_type_0 == None:
                pass

            # Section data encrypted in the second keyblob region
            encrypted_type_1 = sbCmdDict['otfadDecryption'].encrypted_type_1
            encrypted_data_1 = sbCmdDict['otfadDecryption'].encrypted_data_1
            data_location_start_1  = sbCmdDict['otfadDecryption'].data_location_start_1
            data_location_end_1  = sbCmdDict['otfadDecryption'].data_location_end_1
            if encrypted_type_1 == 'encrypt_fill':
                insertedSection += '\t\t' + 'load ' + ('0x%x' %encrypted_data_1) + '.b' + ' > ' + ('0x%x' %data_location_start_1) + '..' + ('0x%x' %data_location_end_1) + ';\n'
            elif encrypted_type_1 == 'encrypt_write':
                insertedSoreces += '    binFile1 = ' + '\"' + encrypted_data_1 + '\"' + ';\n'
                insertedSection += '\t\t' + 'load binFile1 > ' + ('0x%x' %data_location_start_1) + '..' + ('0x%x' %data_location_end_1) + ';\n'
            elif encrypted_type_1 == None:
                pass

            # Section data encrypted in the third keyblob region
            encrypted_type_2 = sbCmdDict['otfadDecryption'].encrypted_type_2
            encrypted_data_2 = sbCmdDict['otfadDecryption'].encrypted_data_2
            data_location_start_2  = sbCmdDict['otfadDecryption'].data_location_start_2
            data_location_end_2  = sbCmdDict['otfadDecryption'].data_location_end_2
            if encrypted_type_2 == 'encrypt_fill':
                insertedSection += '\t\t' + 'load ' + ('0x%x' %encrypted_data_2) + '.b' + ' > ' + ('0x%x' %data_location_start_2) + '..' + ('0x%x' %data_location_end_2) + ';\n'
            elif encrypted_type_2 == 'encrypt_write':
                insertedSoreces += '    binFile2 = ' + '\"' + encrypted_data_2 + '\"' + ';\n'
                insertedSection += '\t\t' + 'load binFile2 > ' + ('0x%x' %data_location_start_2) + '..' + ('0x%x' %data_location_end_2) + ';\n'
            elif encrypted_type_2 == None:
                pass

            # Section data encrypted in the fourth keyblob region
            encrypted_type_3 = sbCmdDict['otfadDecryption'].encrypted_type_3
            encrypted_data_3 = sbCmdDict['otfadDecryption'].encrypted_data_3
            data_location_start_3  = sbCmdDict['otfadDecryption'].data_location_start_3
            data_location_end_3  = sbCmdDict['otfadDecryption'].data_location_end_3
            if encrypted_type_3 == 'encrypt_fill':
                insertedSection += '\t\t' + 'load ' + ('0x%x' %encrypted_data_3) + '.b' + ' > ' + ('0x%x' %data_location_start_3) + '..' + ('0x%x' %data_location_end_3) + ';\n'
            elif encrypted_type_3 == 'encrypt_write':
                insertedSoreces += '    binFile3 = ' + '\"' + encrypted_data_3 + '\"' + ';\n'
                insertedSection += '\t\t' + 'load binFile3 > ' + ('0x%x' %data_location_start_3) + '..' + ('0x%x' %data_location_end_3) + ';\n'
            elif encrypted_type_3 == None:
                pass

            insertedSection += '\t' + '}\n'
            #------------------------------------------------------------------------------
            # Get the values of key encryption key, it's a 32-bit sting
            kek         = sbCmdDict['otfadDecryption'].Key_Encryption_Key
            # Get the location of key encryption key, it's a 32-bit unsigned long address
            kek_located = sbCmdDict['otfadDecryption'].kek_location
            #===================================================================
            # Generate bd contents for keywrap
            #===================================================================
            if (kek != None) and (kek_located != None):
                insertedSection += '\t' + 'keywrap(0){\n'
                insertedSection += '\t\t' + 'load {{' + kek + '}}' + ' > ' + ('0x%x' %kek_located) + ';\n'
                insertedSection += '\t' + '}\n'
                #===================================================================
                # Generate bd contents for programming OTFAD key to IFR
                #===================================================================
                key1, key2, key3, key4 = convert_32bit_key(kek)
                insertedSection += '\t' + 'load ifr ' + ('0x%x' %int(key4, 16)) + ' > ' + '0x20' + ';\n'
                insertedSection += '\t' + 'load ifr ' + ('0x%x' %int(key3, 16)) + ' > ' + '0x21' + ';\n'
                insertedSection += '\t' + 'load ifr ' + ('0x%x' %int(key2, 16)) + ' > ' + '0x22' + ';\n'
                insertedSection += '\t' + 'load ifr ' + ('0x%x' %int(key1, 16)) + ' > ' + '0x23' + ';\n'
            else:
                pass

    # End of for i in range(0, len(sbCommands)) here
    # ------------------------------------------------------------------------------------------------------
    options = options[:-2] + insertedOptions + options[-2:]
    constants = constants[:-2] + insertedConstants + constants[-2:]
    sources = sources[:-2] + insertedSoreces + sources[-2:]
#     keyblob = keyblob[:-2] + insertedKeyblob + keyblob[-2:]
    section = section[:-2] + insertedSection + section[-2:]

    otfadContex0 = otfadContex0[:-2] + insertedContex0 + otfadContex0[-2:]
    otfadContex1 = otfadContex1[:-2] + insertedContex1 + otfadContex1[-2:]
    otfadContex2 = otfadContex2[:-2] + insertedContex2 + otfadContex2[-2:]
    otfadContex3 = otfadContex3[:-2] + insertedContex3 + otfadContex3[-2:]
    keyblob = keyblob[:-2] + otfadContex0 + otfadContex1 + otfadContex2 + otfadContex3 + keyblob[-2:]
    # Get bd file contents
    bdFileContents = options + constants + sources + keyblob + section
    print("%s%s%s" %("-"*20, "BD file contents start", "-"*20))
    print bdFileContents,
    print("%s%s%s" %("-"*20, "BD file contents end", "-"*20))
    # -----------------------------------------------------------------------------------------------
    commonFileName = bl.target.targetName  + '_' + '_'.join(sbCommands) + '_' + encryptionType
    # Create bd file
    bdFileName = commonFileName + '.bd'
    bdFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', bdFileName))
    # Write the contents to the bd file.
    with open(bdFilePath,'w') as fileObj:
        fileObj.write(bdFileContents)
        fileObj.close()
    # -----------------------------------------------------------------------------------------------
    # Create sb file
    sbFileName = commonFileName + '.sb2'
    sbFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', sbFileName))
    # -----------------------------------------------------------------------------------------------
    # Generate elftosb command line.
    elftosb = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', 'elftosb.exe'))
    if encryptionType == 'unencrypted':
        print("Generating unencrypted SB file:")
#         print("%s -V -c %s -o %s" %(os.path.split(elftosb)[1], os.path.split(bdFilePath)[1], os.path.split(sbFilePath)[1]))
        elftosbCommandLine = "%s -V -c %s -o %s" %(elftosb, bdFilePath, sbFilePath)    
    elif encryptionType == 'zeroKeyEncrypted': 
        print("Generating zero key encrypted SB file:")
#         print("%s -V -z -c %s -o %s" %(os.path.split(elftosb)[1], os.path.split(bdFilePath)[1], os.path.split(sbFilePath)[1]))
        elftosbCommandLine = "%s -V -z -c %s -o %s" %(elftosb, bdFilePath, sbFilePath)
    elif encryptionType == 'nonZeroKeyEncrypted':
        keyFilePath = os.path.abspath(os.path.join(bl.vectorsDir, 'mmcau_sb_file', 'key.txt'))
        # write the given key to the key.txt file
        fileObj = file(keyFilePath, 'w+')
        fileObj.write(key)
        fileObj.close()
        print('Generating non-zero key encrypted SB file (key = "%s"):' %key)
#         print("%s -V -k %s -c %s -o %s" %(os.path.split(elftosb)[1], os.path.split(keyFilePath)[1], os.path.split(bdFilePath)[1], os.path.split(sbFilePath)[1]))
        elftosbCommandLine = "%s -V -k %s -c %s -o %s" %(elftosb, keyFilePath, bdFilePath, sbFilePath)
    else:
        raise ValueError('Invalid encryptionType parameter.\n')
    ############################################################################
    
#     elftosbCommandLine = elftosbCommandLine.replace('-V', '-V -d -f kinetisk3')
    print elftosbCommandLine
    p = subprocess.Popen(elftosbCommandLine, stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
    output = p.communicate()[0]
    print output
    print('-'*60 + ' SB file generated successfully.')
    # Delete the bd file and bat file
    os.remove(bdFilePath) 

    return sbFilePath

## @breif This function is to reverse a given 8-bit length string.
#         For example, string = '12345678', then we get result = '78563412'
def reverse_8bit_Str(string):
    result = ''
    for i in range(6,-2,-2):
        result += string[i:i+2]
    return result

## @breif This function is to reverse a given 32-bit length string.
#         For example, string = '000102030405060708090a0b0c0d0e0f', then we get result = '03020100','07060504','0b0a0908','0f0e0d0c'
def convert_32bit_key(key):
    result = []
    for i in range(0,4):
        subKey = key[i*8:(i+1)*8]
        result.append(reverse_8bit_Str(subKey))
    return result[0], result[1], result[2], result[3]
