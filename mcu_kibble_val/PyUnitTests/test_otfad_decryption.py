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
import binascii

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl import bootloader
from common import common_util
from common import user_config_area
from common import flash_config_area
from common import sb_command
from PyUnitTests.test_QSPI import kQspiConfigBlockFile


sbCmdDict = sb_command.sbCmdDict

## @breif This function is to initialize sbCmdDict['otfadDecryption']
# Since there are many parameters in sbCmdDict['otfadDecryption'], and not all the parameters
# are needed in every case, it's necessary to initialize this global variable so that it will no affect
# each test case.
def initOtfadDecryption():
    # Here we initialize all the parameters to None
    sbCmdDict['otfadDecryption'].qspiDemo = None

    sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR = None

    sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX1_END_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX1_KEY = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX1_CTR = None

    sbCmdDict['otfadDecryption'].OTFAD_CTX2_START_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX2_END_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX2_KEY = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX2_CTR = None

    sbCmdDict['otfadDecryption'].OTFAD_CTX3_START_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX3_END_ADDR = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX3_KEY = None
    sbCmdDict['otfadDecryption'].OTFAD_CTX3_CTR = None

    sbCmdDict['otfadDecryption'].encrypted_type_0 = None
    sbCmdDict['otfadDecryption'].encrypted_data_0 = None
    sbCmdDict['otfadDecryption'].data_location_start_1 = None
    sbCmdDict['otfadDecryption'].data_location_end_1 = None

    sbCmdDict['otfadDecryption'].encrypted_type_1 = None
    sbCmdDict['otfadDecryption'].encrypted_data_0 = None
    sbCmdDict['otfadDecryption'].data_location_start_2 = None
    sbCmdDict['otfadDecryption'].data_location_end_2 = None

    sbCmdDict['otfadDecryption'].encrypted_type_2 = None
    sbCmdDict['otfadDecryption'].encrypted_data_0 = None
    sbCmdDict['otfadDecryption'].data_location_start_3 = None
    sbCmdDict['otfadDecryption'].data_location_end_3 = None

    sbCmdDict['otfadDecryption'].encrypted_type_3 = None
    sbCmdDict['otfadDecryption'].encrypted_data_0 = None
    sbCmdDict['otfadDecryption'].data_location_start_4 = None
    sbCmdDict['otfadDecryption'].data_location_end_4 = None

    sbCmdDict['otfadDecryption'].Key_Encryption_Key = None
    sbCmdDict['otfadDecryption'].kek_location = None



## @breif Skip all the tests in this module if the target does not support QSPI or OTFAD
@pytest.fixture(scope = 'module', autouse = True)
def skipif(tgt, request):
    if not tgt.isQspiSupported or not tgt.isDecryptionSupported:
        pytest.skip('%s does not support QSPI.' %(tgt.targetName))


#===============================================================================
# This script is mainly used to test the elftosb encrytion and OTFAD decryption.
# Here developed 6 basic auto test cases:
# Case 1: elftosb.exe cannot encrypt the section data that its start address
#         does not match the start keyblob address. (JIRA issue KBL-1282)
#
# Case 2: elftosb.exe cannot encrypt the filled pattern. (JIRA issue KBL-1282)
#
# Case 3: elftosb.exe cannot encrypt multiple non-contiguous section data
#         within the 512 bytes region. (JIRA issue KBL-1282)
#
# Case 4: Define an overlapped/undefined region, then check if elftosb.exe
#         encrypts the overlapped/undefined region and OTFAD decrypts the
#         overlapped/undefined region.
#
# Case 5: OTFAD decrypts the four independent qspi flash regions.
#
# Case 6: Download encrypted app demo to QSPI flash with encrypted sb file.
#===============================================================================
class TestSecurity:
    @pytest.fixture(autouse = True)
    def setup(self, bl, request):
        common_util.reset_with_check(bl)
        initOtfadDecryption()
        def teardown():
            # We should use JLink to unlock the device so that JLink can connect with the device and reset the device.
            bl.target.unlock()
            common_util.reset_without_check(bl)
        request.addfinalizer(teardown)
    #===========================================================================
    # Case 1: elftosb.exe cannot encrypt the section data that its start address
    #         does not match the start keyblob address. (JIRA issue KBL-1282)
    #===========================================================================
    def test_encrypt_data_with_its_start_address_not_match_keyblob_start_address(self, bl):
        # Get the path of the QCB bin file path
        qcbBinFile = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))

        # Only use one keyblob region
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR = 0x68001000   # keyblob start address
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR = 0x6800FFFF     # keyblob end address
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR = "FFFFFFFFFFFFFFFF"

        sbCmdDict['otfadDecryption'].encrypted_type_0 = 'encrypt_write'
        # Encrypt the qspi config block data. Here can be any bin files
        sbCmdDict['otfadDecryption'].encrypted_data_0 = qcbBinFile
        # Get the encrypted data size
        data_size = os.path.getsize(sbCmdDict['otfadDecryption'].encrypted_data_0)
        # The start address does not match the keyblob start address
        sbCmdDict['otfadDecryption'].data_location_start_0 = sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR + 0x1000
        sbCmdDict['otfadDecryption'].data_location_end_0 = sbCmdDict['otfadDecryption'].data_location_start_0 + data_size

        # Enable the qspi in bd file
        sbCmdDict['enableQspi'].qspiConfigBlock = qcbBinFile
        sbCmdDict['enableQspi'].qcbLocation = 0x20000000
        # Erase qspi flash in bd file
        sbCmdDict['flashEraseRegion'].startAddress = 0x68000000
        sbCmdDict['flashEraseRegion'].endAddress = 0x68006000

        # Generate sb file according to the parameters
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'enableQspi', 'flashEraseRegion', 'otfadDecryption')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success

        plaintext = qcbBinFile
        # Read back the ciphertext
        ciphertext = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(sbCmdDict['otfadDecryption'].data_location_start_0, data_size, ciphertext)
        assert status == bootloader.status.kStatus_Success

        # The plaintext should not be the same with the ciphertext
        with open(plaintext, 'rb') as plain_Obj:
            plain_data = plain_Obj.read()
            plain_Obj.close()
        with open(ciphertext, 'rb') as cipher_Obj:
            ciphter_data = cipher_Obj.read()
            cipher_Obj.close()

        assert plain_data != ciphter_data

    #===========================================================================
    # Case 2: elftosb.exe cannot encrypt the filled pattern.
    #         (JIRA issue KBL-1282)
    #===========================================================================
    def test_encrypt_filled_pattern(self, bl):
        # Get the path of the QCB bin file path
        qcbBinFile = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))

        # Only use one keyblob region
        qspi_config_block = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR = 0x68001000   # keyblob start address
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR = 0x68001FFF     # keyblob end address
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR = "FFFFFFFFFFFFFFFF"

        sbCmdDict['otfadDecryption'].encrypted_type_0 = 'encrypt_fill'
        # Encrypt the qspi config block data
        sbCmdDict['otfadDecryption'].encrypted_data_0 = 0x12  # filled pattern
        # Fill 1KB data pattern to have a test
        data_size = 0x400
        # Let the start address matchs the keyblob start address
        sbCmdDict['otfadDecryption'].data_location_start_0 = sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR
        sbCmdDict['otfadDecryption'].data_location_end_0 = sbCmdDict['otfadDecryption'].data_location_start_0 + data_size

        # Enable the qspi in bd file
        sbCmdDict['enableQspi'].qspiConfigBlock = qcbBinFile
        sbCmdDict['enableQspi'].qcbLocation = 0x20000000
        # Erase qspi flash in bd file
        sbCmdDict['flashEraseRegion'].startAddress = 0x68000000
        sbCmdDict['flashEraseRegion'].endAddress = 0x68006000

        # Generate sb file according to the parameters
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'enableQspi', 'flashEraseRegion', 'otfadDecryption')
        status, results = bl.receive_sb_file(sbFilePath)
        assert status == bootloader.status.kStatus_Success

        plaintext = common_util.generate_file_according_to_filled_pattern(bl, sbCmdDict['otfadDecryption'].encrypted_data_0, 'byte', data_size)
        # Read back the ciphertext
        ciphertext = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
        status, results = bl.read_memory(sbCmdDict['otfadDecryption'].data_location_start_0, data_size, ciphertext)
        assert status == bootloader.status.kStatus_Success


        # The plaintext should not be the same with the ciphertext
        with open(plaintext, 'rb') as plain_Obj:
            plain_data = plain_Obj.read()
            plain_Obj.close()
        with open(ciphertext, 'rb') as cipher_Obj:
            ciphter_data = cipher_Obj.read()
            cipher_Obj.close()

        assert plain_data != ciphter_data

    #===========================================================================
    # Case 3: elftosb.exe cannot encrypt multiple non-contiguous section data
    #         within the 512 bytes region. (JIRA issue KBL-1282)
    #===========================================================================
    def test_encrypt_noncontiguous_sections_within_512bytes(self, bl):
        # Get the path of the QCB bin file path
        qcbBinFile = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))

        # Use two keyblob regions to encrypt two non-contiguous setion data
        # The first keyblob region
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR = 0x68002300   # keyblob start address
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR = 0x680023FF     # keyblob end address
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR = "FFFFFFFFFFFFFFFF"

        # Section data 0 needs to be encrypted
        sbCmdDict['otfadDecryption'].encrypted_type_0 = 'encrypt_write'
        sbCmdDict['otfadDecryption'].encrypted_data_0 = qcbBinFile    # Here can be any other bin files
        # Only encrypt 16 bytes data
        encrypted_data_size_0 = 16
        sbCmdDict['otfadDecryption'].data_location_start_0 = sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR
        sbCmdDict['otfadDecryption'].data_location_end_0 = sbCmdDict['otfadDecryption'].data_location_start_0 + encrypted_data_size_0

        # The second keyblob region
        sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR = 0x68002400   # keyblob start address
        sbCmdDict['otfadDecryption'].OTFAD_CTX1_END_ADDR = 0x680027FF     # keyblob end address
        sbCmdDict['otfadDecryption'].OTFAD_CTX1_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        sbCmdDict['otfadDecryption'].OTFAD_CTX1_CTR = "FFFFFFFFFFFFFFFF"
        # Section data 1 needs to be encrypted
        sbCmdDict['otfadDecryption'].encrypted_type_1 = 'encrypt_write'
        sbCmdDict['otfadDecryption'].encrypted_data_1 = qcbBinFile    # Here can be any other bin files
        # Only encrypt 16 bytes data
        encrypted_data_size_1 = 16
        sbCmdDict['otfadDecryption'].data_location_start_1 = sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR
        sbCmdDict['otfadDecryption'].data_location_end_1 = sbCmdDict['otfadDecryption'].data_location_start_1 + encrypted_data_size_1

        # Enable the qspi in bd file
        sbCmdDict['enableQspi'].qspiConfigBlock = qcbBinFile
        sbCmdDict['enableQspi'].qcbLocation = 0x20000000
        # Erase qspi flash in bd file
        sbCmdDict['flashEraseRegion'].startAddress = 0x68000000
        sbCmdDict['flashEraseRegion'].endAddress = 0x68004000

        # Generate sb file according to the parameters
        sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'enableQspi', 'flashEraseRegion', 'otfadDecryption')
        status, results = bl.receive_sb_file(sbFilePath)
        # Obviously, the qspi flash 0x68002400 ~ 0x68002500 is cumulative programmed. So send this sb file to target will get QSPI Flash Command Failure
        assert status == bootloader.status.kStatus_Success

        # To do
        # If this issue needs to be fixed, should verify the encryptions here.

    #===========================================================================
    # Case 4: Define an overlapped/undefined region, then check if elftosb.exe
    #         encrypts the overlapped/undefined region and OTFAD decrypts the
    #         overlapped/undefined region.
    #===========================================================================
    def test_encrypt_decrypt_overlapped_region(self, bl):
        # Get the led demo that running in the qspi flash
        app_exists, qspiDemo = common_util.get_led_demo_path(bl, running_in_qspi_flash = True)
        if app_exists == False:
            # if not exist the app file, should mark the case as SKIPPED but not FAILED
            print qspiDemo
            pytest.skip("\nDo not find the app demo srec file.")
        else:
            # Get the path of the QCB bin file path
            qcbBinFile = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))

            sbCmdDict['otfadDecryption'].qspiDemo = qspiDemo
            # Check if the led demo code is located at 0x68001000
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR = 0x68001000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR = 0x68001FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR = "FFFFFFFFFFFFFFFF"
            # Encrypt app demo
            sbCmdDict['otfadDecryption'].encrypted_type_0 = 'encrypt_appSrecFile'

            # The second keyblob region
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR = 0x68002000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_END_ADDR = 0x68002FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_CTR = "FFFFFFFFFFFFFFFF"
            # Encrypt data in the overlapped region (0x68002000 ~ 0x68002FFF)
            sbCmdDict['otfadDecryption'].encrypted_type_1 = 'encrypt_write'
            sbCmdDict['otfadDecryption'].encrypted_data_1 = qcbBinFile # Here can be any other bin files
            sbCmdDict['otfadDecryption'].data_location_start_1 = sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR
            encrypt_data_size_1 = os.path.getsize(sbCmdDict['otfadDecryption'].encrypted_data_1)
            sbCmdDict['otfadDecryption'].data_location_end_1 = sbCmdDict['otfadDecryption'].data_location_start_1 + encrypt_data_size_1

            # The third keyblob region, which is overlapped with the second keyblob region
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_START_ADDR = 0x68002000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_END_ADDR = 0x68003FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_CTR = "FFFFFFFFFFFFFFFF"
            # Encrypt data in the undefined region
            sbCmdDict['otfadDecryption'].encrypted_type_2 = 'encrypt_write'
            sbCmdDict['otfadDecryption'].encrypted_data_2 = qcbBinFile # Here can be any other bin files
            sbCmdDict['otfadDecryption'].data_location_start_2 = 0x68004000 # undefined keyblob region
            encrypt_data_size_2 = os.path.getsize(sbCmdDict['otfadDecryption'].encrypted_data_2)
            sbCmdDict['otfadDecryption'].data_location_end_2 = sbCmdDict['otfadDecryption'].data_location_start_2 + encrypt_data_size_2


            sbCmdDict['otfadDecryption'].Key_Encryption_Key = "5391e2d3c4b5a69788796a5b4c3d2e1f"
            # Check if the keyBlobPointer is 0x1000 in the BCA
            sbCmdDict['otfadDecryption'].kek_location = 0x1000

            # Enable the qspi in bd file
            sbCmdDict['enableQspi'].qspiConfigBlock = qcbBinFile
            sbCmdDict['enableQspi'].qcbLocation = 0x20000000
            # Erase qspi flash in bd file
            sbCmdDict['flashEraseRegion'].startAddress = 0x68000000
            sbCmdDict['flashEraseRegion'].endAddress = 0x68005000

            # Generate encrypted sb file and send the sb file to target
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'enableQspi', 'flashEraseRegion', 'otfadDecryption')
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatus_Success

            # ----------------------------------------------------------------------------------------------
            # Check if elftosb.exe encrypts the overlapped region (will encrypt), and
            # check if the elftosb.exe encrypts the undefined region (will not encrypt)
            # ----------------------------------------------------------------------------------------------
            # 1. Get the original data that will be encrypted in the overlapped region
            with open(sbCmdDict['otfadDecryption'].encrypted_data_1, 'rb') as fileObj:
                original_data_overlapped = fileObj.read()
                fileObj.close()

            # 2. Get data from the overlapped region after encryption
            binFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
            status, results = bl.read_memory(sbCmdDict['otfadDecryption'].data_location_start_1, encrypt_data_size_1, binFile)
            assert status == bootloader.status.kStatus_Success
            with open(binFile, 'rb') as fileObj:
                encrypted_data_overlapped = fileObj.read()
                fileObj.close()
            # elftosb encrypts the overlapped region, so encrypted_data_overlapped is not equal to original_data_overlapped
            assert encrypted_data_overlapped != original_data_overlapped

            # 3. Get the original data that will be encrypted in the undefined region
            with open(sbCmdDict['otfadDecryption'].encrypted_data_2, 'rb') as fileObj:
                original_data_undefined = fileObj.read()
                fileObj.close()

            # 4. Get data from the undefined region after encryption
            binFile = os.path.join(bl.vectorsDir, 'read_data_from_memory.bin')
            status, results = bl.read_memory(sbCmdDict['otfadDecryption'].data_location_start_2, encrypt_data_size_2, binFile)
            assert status == bootloader.status.kStatus_Success
            with open(binFile, 'rb') as fileObj:
                encrypted_data_undefined = fileObj.read()
                fileObj.close()
            # elftosb does not encrypt the undefined region, so encrypted_data_undefined is equal to original_data_undefined
            assert encrypted_data_undefined == original_data_undefined

            # 5. Reset the target we can see the led blinking. OTFAD starts to decrypt after app running.
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            # Let led blink for some time so that we can see the correct phenomenon.
            time.sleep(3)

            # 6. Now communicate with bootloader, it has no response which means OTFAD decrypts the app successfully
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            assert status != bootloader.status.kStatus_Success

            # 7. Get data from the overlapped region after decryption
            decrypted_data_overlapped = bl.target.read(sbCmdDict['otfadDecryption'].data_location_start_1, encrypt_data_size_1)
            # OTFAD does not decrypt the overlapped region, so decrypted_data_overlapped is equal to encrypted_data_overlapped
            assert decrypted_data_overlapped == encrypted_data_overlapped

            # 8. Get data from the undefined region after decryption
            decrypted_data_undefined = bl.target.read(sbCmdDict['otfadDecryption'].data_location_start_2, encrypt_data_size_2)
            # OTFAD does not decrypt the undefined region, so decrypted_data_undefined is equal to encrypted_data_undefined
            assert decrypted_data_undefined == encrypted_data_undefined

    #===========================================================================
    # Case 5: OTFAD decrypts the four independent qspi flash regions
    #===========================================================================
    def test_decrypt_four_nonoverlapped_regions(self, bl):
        # Get the led demo that running in the qspi flash
        app_exists, qspiDemo = common_util.get_led_demo_path(bl, 'app_srec', running_in_qspi_flash = True)
        if app_exists == False:
            # if not exist the app file, should mark the case as SKIPPED but not FAILED
            print qspiDemo
            pytest.skip("\nDo not find the app demo srec file.")
        else:
            # Get the path of the QCB bin file path
            qcbBinFile = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))

            sbCmdDict['otfadDecryption'].qspiDemo = qspiDemo
            # The first keyblob region. App demo is encrypted and decrypted in this region.
            # Check if the app demo code is located at 0x68001000.
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR = 0x68001000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR = 0x68001FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR = "FFFFFFFFFFFFFFFF"

            sbCmdDict['otfadDecryption'].encrypted_type_0 = 'encrypt_appSrecFile'

            # The second keyblob region
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR = 0x68002000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_END_ADDR = 0x68002FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX1_CTR = "FFFFFFFFFFFFFFFF"

            sbCmdDict['otfadDecryption'].encrypted_type_1 = 'encrypt_write'
            sbCmdDict['otfadDecryption'].encrypted_data_1 = qcbBinFile # Here can be any other bin files
            sbCmdDict['otfadDecryption'].data_location_start_1 = sbCmdDict['otfadDecryption'].OTFAD_CTX1_START_ADDR
            encrypt_data_size_1 = os.path.getsize(sbCmdDict['otfadDecryption'].encrypted_data_1)
            sbCmdDict['otfadDecryption'].data_location_end_1 = sbCmdDict['otfadDecryption'].data_location_start_1 + encrypt_data_size_1

            # The third keyblob region
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_START_ADDR = 0x68003000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_END_ADDR = 0x68003FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX2_CTR = "FFFFFFFFFFFFFFFF"

            sbCmdDict['otfadDecryption'].encrypted_type_2 = 'encrypt_write'
            sbCmdDict['otfadDecryption'].encrypted_data_2 = qcbBinFile # Here can be any other bin files
            sbCmdDict['otfadDecryption'].data_location_start_2 = sbCmdDict['otfadDecryption'].OTFAD_CTX2_START_ADDR
            encrypt_data_size_2 = os.path.getsize(sbCmdDict['otfadDecryption'].encrypted_data_2)
            sbCmdDict['otfadDecryption'].data_location_end_2 = sbCmdDict['otfadDecryption'].data_location_start_2 + encrypt_data_size_2

            # The fourth keyblob region
            sbCmdDict['otfadDecryption'].OTFAD_CTX3_START_ADDR = 0x68004000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX3_END_ADDR = 0x68004FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX3_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX3_CTR = "FFFFFFFFFFFFFFFF"

            sbCmdDict['otfadDecryption'].encrypted_type_3 = 'encrypt_write'
            sbCmdDict['otfadDecryption'].encrypted_data_3 = qcbBinFile # Here can be any other bin files
            sbCmdDict['otfadDecryption'].data_location_start_3 = sbCmdDict['otfadDecryption'].OTFAD_CTX3_START_ADDR
            encrypt_data_size_3 = os.path.getsize(sbCmdDict['otfadDecryption'].encrypted_data_3)
            sbCmdDict['otfadDecryption'].data_location_end_3 = sbCmdDict['otfadDecryption'].data_location_start_3 + encrypt_data_size_3

            sbCmdDict['otfadDecryption'].Key_Encryption_Key = "5391e2d3c4b5a69788796a5b4c3d2e1f"
            # Check if the keyBlobPointer is 0x1000 in the BCA
            sbCmdDict['otfadDecryption'].kek_location = 0x1000

            # Enable the qspi in bd file
            sbCmdDict['enableQspi'].qspiConfigBlock = qcbBinFile
            sbCmdDict['enableQspi'].qcbLocation = 0x20000000
            # Erase qspi flash in bd file
            sbCmdDict['flashEraseRegion'].startAddress = 0x68000000
            sbCmdDict['flashEraseRegion'].endAddress = 0x68005000

            # Generate encrypted sb file and send the sb file to target
            sbFilePath = sb_command.generate_sb_file(bl, 'unencrypted', '', 'enableQspi', 'flashEraseRegion', 'otfadDecryption')
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatus_Success

            # ---------------------------------------------------------------------------------------------------
            # Check if the data in these four independent keyblob regions can be successfully decrypted by OTFAD.
            # ---------------------------------------------------------------------------------------------------
            # 1. Reset the target, the led will blink, which means OTFAD decrypts the first keyblob region data.
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(3)

            # 2. Communicate with bootloader, it will have no response.
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            assert status != bootloader.status.kStatus_Success

            # 3. Use JLink to read data from the other three keyblob regions
            decrypted_data_1 = bl.target.read(sbCmdDict['otfadDecryption'].data_location_start_1, encrypt_data_size_1)
            decrypted_data_2 = bl.target.read(sbCmdDict['otfadDecryption'].data_location_start_2, encrypt_data_size_2)
            decrypted_data_3 = bl.target.read(sbCmdDict['otfadDecryption'].data_location_start_3, encrypt_data_size_3)

            # 4. Compare the decryption data with the pliantext
            with open(qcbBinFile, 'rb') as fileObj:
                plaintext = fileObj.read()
                fileObj.close()
            assert decrypted_data_1 == plaintext
            assert decrypted_data_2 == plaintext
            assert decrypted_data_3 == plaintext

    #===========================================================================
    # Case 6: Download encrypted app demo to QSPI flash with encrypted sb file.
    #===========================================================================
    def test_download_encrypted_app_with_encrypted_sb_file(self, bl):
        # Get the led demo that running in the qspi flash
        app_exists, qspiDemo = common_util.get_led_demo_path(bl, 'app_srec', running_in_qspi_flash = True)
        if app_exists == False:
            # if not exist the app file, should mark the case as SKIPPED but not FAILED
            print qspiDemo
            pytest.skip("\nDo not find the app demo srec file.")
        else:
            # Get the path of the QCB bin file path
            qcbBinFile = os.path.abspath(os.path.join(bl.vectorsDir, 'QSPI', kQspiConfigBlockFile))
            sbCmdDict['otfadDecryption'].qspiDemo = qspiDemo
            # Check if the led demo code is located at 0x68001000
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_START_ADDR = 0x68001000   # keyblob start address
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_END_ADDR = 0x68001FFF     # keyblob end address
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_KEY = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
            sbCmdDict['otfadDecryption'].OTFAD_CTX0_CTR = "FFFFFFFFFFFFFFFF"

            sbCmdDict['otfadDecryption'].encrypted_type_0 = 'encrypt_appSrecFile'

            sbCmdDict['otfadDecryption'].Key_Encryption_Key = "00f1e2d3c4b5a69788796a5b4c3d2e1f"
            # Check if the keyBlobPointer is 0x1000 in the BCA
            sbCmdDict['otfadDecryption'].kek_location = 0x1000

            # Enable the qspi in bd file
            sbCmdDict['enableQspi'].qspiConfigBlock = qcbBinFile
            sbCmdDict['enableQspi'].qcbLocation = 0x20000000
            # Erase qspi flash in bd file
            sbCmdDict['flashEraseRegion'].startAddress = 0x68000000
            sbCmdDict['flashEraseRegion'].endAddress = 0x68002000

            # Erase the whole flash
            status, results = bl.flash_erase_all()
            assert status == bootloader.status.kStatus_Success

            # Generate encrypted sb file
            sb_file_encryption_key = '000102030405060708090a0b0c0d0e0f'
            sbFilePath = sb_command.generate_sb_file(bl, 'nonZeroKeyEncrypted', sb_file_encryption_key, 'enableQspi', 'flashEraseRegion', 'otfadDecryption')

            # Get the sb file IFR key according to the given sb_file_encryption_key
            key1, key2, key3, key4 = sb_command.convert_32bit_key(sb_file_encryption_key)

            # Program the key to IFR
            key = [key1, key2, key3, key4]
            for i in range(0, len(key)):
                status, results = bl.flash_program_once(0x30 + i, 4, key[i])
                assert status == bootloader.status.kStatus_Success

            # Reset the target and let it be in secure
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            time.sleep(2)

            # Check if flash is in secure
            status, results = bl.get_property(bootloader.properties.kPropertyTag_FlashSecurityState)
            assert status == bootloader.status.kStatus_Success
            assert results[0] == 1

            # Send the encrypted sb file to target
            status, results = bl.receive_sb_file(sbFilePath)
            assert status == bootloader.status.kStatus_Success

            # Reset the target we can see the led blinking
            status, results = bl.reset()
            assert status == bootloader.status.kStatus_Success
            # Let led blink for some time so that we can see the correct phenomenon.
            time.sleep(3)

            # Now communicate with bootloader, it has no response which means OTFAD decrypts the app successfully
            status, results = bl.get_property(bootloader.properties.kPropertyTag_CurrentVersion)
            assert status != bootloader.status.kStatus_Success

