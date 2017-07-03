############################################################################################           
               ##API arguments test
#Write demo to 0x20000000, after executing API, status will be wrote to fixed address 0x20003000
#cases                                    returned value
#options.version = 2                      4        
#options.buffer = NULL                    4 
#options.bufferLength = 0                 4
#options.op = kRomOperation               4                  
                          
               ##SB loader test
#Write demo to 0x20000000, after executing API, status will be wrote to fixed address 0x20003000
#if run sb loader successfully, SB content 0x1234 will be wrote 0xa000-0xb000
#build number of SB file is 0xab
#cases                                    returned value
#wrong SBKEK                              10101
#sb loader successfully                   0
#sb buildnumber rollback(>0xab)           10115     
#region out of sb data                    10200

               ##authentication
#Write demo to 0x20000000, after executing API, status will be wrote to fixed address 0x20003000
#Write signed SB file 0xa000 and RTKH to 0x94-0x97, demo will autheticate content from this address
#build number of SB file is 0xab
#cases                                            returned value
#NO CA                                            0
#CA                                               10703
#CA + CA + NO CA use Root1                        0
#CA + CA + NO CA use Root2                        0
#CA + CA + NO CA use Root3                        0
#CA + CA + NO CA use Root4                        0
#CA + CA + NO CA revoke Root1                     10711
#CA + CA + NO CA revoke Root2                     10711
#CA + CA + NO CA revoke Root3                     10711
#CA + CA + NO CA revoke Root4                     10711
#CA + CA + NO CA sign failed                      10706
#CA + CA + NO CA with key 2048, profile 4096      10707
#CA + CA + NO CA with key 2048, profile 2048      0
#API set build number as 0xac                     10704

import pytest
import sys
import os
import time
import random
import array
import struct
filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env
from fsl import bootloader
from common import common_util

#signed SB files
SBOneCertNoCA            = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , '1_signed_no_ca.sb2')
SBOneCertCA              = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , '1_signed_ca.sb2')
SBCACANoCARoot1          = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , 'signed_ca_ca_noca_root1_used.sb2')
SBCACANoCARoot1Failed    = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , 'signed_ca_ca_noca_root1_used_failed.sb2')
SBCACANoCARoot2          = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , 'signed_ca_ca_noca_root2_used.sb2')
SBCACANoCARoot3          = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , 'signed_ca_ca_noca_root3_used.sb2')
SBCACANoCARoot4          = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , 'signed_ca_ca_noca_root4_used.sb2')
SB2048CACANoCARoot1      = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'signedSB' , 'signed_ca_ca_noca_root1_used2048.sb2')


#files for testing API
SBWrongKeyFile           = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_encrypted_wrong_key.bin')
SBsuccessFile            = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_success.bin')
SBRollBackFile           = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_roll_back.bin')
SBRegionOutsideFile      = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_region_outside.bin')
#files for testing API M0
SBWrongKeyFileM0         = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_encrypted_wrong_key_M0.bin')
SBsuccessFileM0          = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_success_M0.bin')
SBRollBackFileM0         = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_roll_back_M0.bin')
SBRegionOutsideFileM0    = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_LS_region_outside_M0.bin')
#files for testing API authentication
APIAuthPass              = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_pass.bin')
APIAuthPassKey           = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_pass_key.bin')
APIProfile2048           = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_profile_2048.bin')
APIProfileBuildNumber0xac= os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_build_number_0xac.bin')
#files for testing API authentication M0
APIAuthPassM0              = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_pass_M0.bin')
APIAuthPassKeyM0           = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_pass_key_M0.bin')
APIProfile2048M0           = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_profile_2048_M0.bin')
APIProfileBuildNumber0xacM0= os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_auth_build_number_0xac_M0.bin')
#files for testing API argument
APIBufferlength0         = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_buffer_length_0.bin')
APIBufferNull            = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_buffer_null.bin')
APIOption3               = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_option_3.bin')
APIVersion2              = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'rom_api_demo_version_2.bin')








SBDataFile     = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'API_file' , 'SBData.bin')

#file for read data from 0x20003000
readFile = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'read_data_from_memory.bin')


 #init an array with 0xff for receiving sp and stack from demo
def get_PC_stack_address_from_demo(demoFile):
    TempArr = array.array('c',[chr(0xff)])
    with open(demoFile, 'rb') as file:
        TempArr = file.read(8)
        file.close()        
    Stack = (ord(TempArr[3])<<24) + (ord(TempArr[2])<<16) + (ord(TempArr[1])<<8) + (ord(TempArr[0]))
    PC    = (ord(TempArr[7])<<24) + (ord(TempArr[6])<<16) + (ord(TempArr[5])<<8) + (ord(TempArr[4]))
    print 'stackpointer is: %s' %hex(Stack)
    print 'PC is: %s' %hex(PC)
    return Stack, PC

#get data from 0x20003000
def get_data(bl, address, length):
    status, results = bl.read_memory(address, length, readFile)
    assert status == bootloader.status.kStatus_Success
    readBackBytes = None
    with open (readFile, 'rb') as file:
        readBackBytes = file.read(int(length))
        file.close()
    if length == 1:
        format = 'B'
    elif length == 2:
        format = 'H'
    elif length == 4:
        format = 'L'
    assert readBackBytes != None
    value, = struct.unpack(format, readBackBytes)
    print value
    return value

#check data executed by SB loader
def read_verify_SB(bl):
    #read data from 0xa000,length 0x1000.If SB file loaded successfully, 0xa000 with length 0x1000 will be filled with data 0x1234
    status, results = bl.read_memory(0xa000, 0x1000, readFile)
    assert status == bootloader.status.kStatus_Success
    # The two data files should be the same.
    with open(readFile, 'rb') as fileObj_readFile:
       data1 = fileObj_readFile.read()
       fileObj_readFile.close()
    with open(SBDataFile, 'rb') as fileObj_ExpectedFile:
       data2 = fileObj_ExpectedFile.read()
       fileObj_ExpectedFile.close()
    assert data1 == data2
    
#Write RKTH to IFR
def write_RKTH(bl, key1, key2, key3, key4):
    status, results = bl.flash_program_once(0x94, 8, key1, 'msb')
    assert status == bootloader.status.kStatus_Success
    status, results = bl.flash_program_once(0x95, 8, key2, 'msb')
    assert status == bootloader.status.kStatus_Success
    status, results = bl.flash_program_once(0x96, 8, key3, 'msb')
    assert status == bootloader.status.kStatus_Success
    status, results = bl.flash_program_once(0x97, 8, key4, 'msb')
    assert status == bootloader.status.kStatus_Success
    

class TestAPIArguments:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)
    def test_API_argument_buffer_length_0(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIBufferlength0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIBufferlength0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1) 
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_InvalidArgument
               
    def test_API_argument_buffer_null(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIBufferNull)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIBufferNull)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_InvalidArgument
                
    def test_API_argument_option_3(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIOption3)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIOption3)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_InvalidArgument
                
    def test_API_argument_version_2(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIVersion2)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIVersion2)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_InvalidArgument
              
               
class TestSBLoaderM4:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)
                       
    def test_SBLoader_wrong_key(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, SBWrongKeyFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBWrongKeyFile)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
 
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusRomLdrSignature
               
    def test_SBLoader_success(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, SBsuccessFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBsuccessFile)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
 
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
        #check data generated by sbloader
        read_verify_SB(bl)
               
               
    def test_SBLoader_rollBack(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, SBRollBackFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBRollBackFile)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #reset the board, after execute the app,blhost can't communicate with the target
        common_util.reset_without_check(bl)
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusRomLdrRollbackBlocked
      
    def test_SBLoader_region_out_of_SBData(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, SBRegionOutsideFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBRegionOutsideFile)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusMemoryRangeInvalid
       
    def test_SBLoader_region_out_of_SBData(self,bl):
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, SBRegionOutsideFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBRegionOutsideFile)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusMemoryRangeInvalid
    
    
class TestAuthenticationM4:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)
                      
    def test_auth_success_NoCA(self,bl):
        #write RKTH to target
        write_RKTH(bl, 'A895A3C3F9D0BAB7', '7766D2ED9803C158', '35EBC657960DF4A5', 'E522CF026993F0AF')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBOneCertNoCA)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
             
    def test_auth_failed_CA(self,bl):
        #write RKTH to target
        write_RKTH(bl, '493B3B6D056FDD3A ', 'EC8E05130111F87A', 'C9C855D978B8A256', '074ABB3CB1FFAB26')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBOneCertCA)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
    
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidCertFormat
             
    def test_auth_success_Root1_used(self,bl):
        #write RKTH to target
        write_RKTH(bl, 'E079C6534806E444', 'EDC5A4F829AC3BC9', '6F10E6BE04CF33B4', 'B910781DDEF56552')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
             
    def test_auth_success_Root2_used(self,bl):
        #write RKTH to target
        write_RKTH(bl, 'C3BAE4955A8C7630', '68378A7D692B7175', 'E419B1B789168FA0', '82DC1F20B760AF63')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot2)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
             
    def test_auth_success_Root3_used(self,bl):
        #write RKTH to target
        write_RKTH(bl, 'CAB8DBF2B5598390', '231C18AA16FC3430', '3E005C246CB901AF', '16C7F873BD021A1F')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot3)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
      
    def test_auth_success_Root4_used(self,bl):
        #write RKTH to target
        write_RKTH(bl, '285ED21C61ED2AB5', '7BF3B4CD41066B0A', 'BE0841E680EE9362', 'D1A0D7D13BA83728')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot4)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
     
    def test_auth_success_Root1_revoked(self,bl):
        #write RKTH to target
        write_RKTH(bl, 'E079C6534806E444', 'EDC5A4F829AC3BC9', '6F10E6BE04CF33B4', 'B910781DDEF56552')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert
              
    def test_auth_success_Root2_revoked(self,bl):
        #write RKTH to target
        write_RKTH(bl, 'C3BAE4955A8C7630', '68378A7D692B7175', 'E419B1B789168FA0', '82DC1F20B760AF63')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot2)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert
              
    def test_auth_success_Root3_revoked(self,bl):
        #write RKTH to target
        write_RKTH(bl, 'CAB8DBF2B5598390', '231C18AA16FC3430', '3E005C246CB901AF', '16C7F873BD021A1F')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot3)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert
       
    def test_auth_success_Root4_revoked(self,bl):
        #write RKTH to target
        write_RKTH(bl, '285ED21C61ED2AB5', '7BF3B4CD41066B0A', 'BE0841E680EE9362', 'D1A0D7D13BA83728')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot4)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert  
     
    def test_auth_failed(self,bl):
       #write RKTH to target
       write_RKTH(bl, 'E079C6534806E444', 'EDC5A4F829AC3BC9', '6F10E6BE04CF33B4', 'B910781DDEF56552')
       #write signed SB file to 0xa000
       status,result = bl.write_memory(0xa000,SBCACANoCARoot1Failed)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #write demo to 0x20000000
       status,result = bl.write_memory(0x20000000, APIAuthPass)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #execute demo
       StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
       status, result = bl.execute(PC, 0, StackPointer)
       assert status == bootloader.status.kStatus_Success
       
       time.sleep(1)   
       #get data from 0x20003000
       data = get_data(bl, 0x20003000, 4)
       assert data ==bootloader.status.kStatus_Authentication_SignatureVeriyFailed
           
    def test_auth_pass_key(self,bl):
       #write signed SB file to 0xa000
       status,result = bl.write_memory(0xa000,SBCACANoCARoot1)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #write demo to 0x20000000
       status,result = bl.write_memory(0x20000000, APIAuthPassKey)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #execute demo
       StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassKey)
       status, result = bl.execute(PC, 0, StackPointer)
       assert status == bootloader.status.kStatus_Success

       time.sleep(1)   
       #get data from 0x20003000
       data = get_data(bl, 0x20003000, 4)
       assert data ==bootloader.status.kStatus_Success
            
            
    def test_auth_profile4096(self,bl):
        #write RKTH to target
        write_RKTH(bl, '2371B3DE6ED3622A', '198A735F5D6F2BBC', '7335FADE59ED7213', '03061F552DEDF778')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SB2048CACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIAuthPass)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPass)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidKeyLength
             
    def test_auth_profile2048(self,bl):
        #write RKTH to target
        write_RKTH(bl, '2371B3DE6ED3622A', '198A735F5D6F2BBC', '7335FADE59ED7213', '03061F552DEDF778')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SB2048CACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIProfile2048)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIProfile2048)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
            
    def test_auth_rollBack(self,bl):
        #write RKTH to target
        write_RKTH(bl, '2371B3DE6ED3622A', '198A735F5D6F2BBC', '7335FADE59ED7213', '03061F552DEDF778')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SB2048CACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write demo to 0x20000000
        status,result = bl.write_memory(0x20000000, APIProfileBuildNumber0xac)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIProfileBuildNumber0xac)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success

        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_DeniedRollBack
         
class TestSBLoaderM0:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)
                      
    def test_SBLoader_wrong_key(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, SBWrongKeyFileM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBWrongKeyFileM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusRomLdrSignature
              
    def test_SBLoader_success(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, SBsuccessFileM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBsuccessFileM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
        #check data generated by sbloader
        read_verify_SB(bl)
              
              
    def test_SBLoader_rollBack(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, SBRollBackFileM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBRollBackFileM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusRomLdrRollbackBlocked
     
    def test_SBLoader_region_out_of_SBData(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, SBRegionOutsideFileM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBRegionOutsideFileM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusMemoryRangeInvalid
      
    def test_SBLoader_region_out_of_SBData(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, SBRegionOutsideFileM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(SBRegionOutsideFileM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatusMemoryRangeInvalid
  
  
class TestAuthenticationM0:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.erase_all_available_flash_unsecure(bl)
        def teardown():
            common_util.erase_all_available_flash_unsecure(bl)
        request.addfinalizer(teardown)
                    
    def test_auth_success_NoCA(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, 'A895A3C3F9D0BAB7', '7766D2ED9803C158', '35EBC657960DF4A5', 'E522CF026993F0AF')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBOneCertNoCA)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
           
    def test_auth_failed_CA(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, '493B3B6D056FDD3A ', 'EC8E05130111F87A', 'C9C855D978B8A256', '074ABB3CB1FFAB26')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBOneCertCA)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidCertFormat
           
    def test_auth_success_Root1_used(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, 'E079C6534806E444', 'EDC5A4F829AC3BC9', '6F10E6BE04CF33B4', 'B910781DDEF56552')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
            
    def test_auth_success_Root2_used(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, 'C3BAE4955A8C7630', '68378A7D692B7175', 'E419B1B789168FA0', '82DC1F20B760AF63')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot2)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
            
    def test_auth_success_Root3_used(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, 'CAB8DBF2B5598390', '231C18AA16FC3430', '3E005C246CB901AF', '16C7F873BD021A1F')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot3)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
     
    def test_auth_success_Root4_used(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, '285ED21C61ED2AB5', '7BF3B4CD41066B0A', 'BE0841E680EE9362', 'D1A0D7D13BA83728')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot4)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
    
    def test_auth_success_Root1_revoked(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, 'E079C6534806E444', 'EDC5A4F829AC3BC9', '6F10E6BE04CF33B4', 'B910781DDEF56552')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9c, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert
             
    def test_auth_success_Root2_revoked(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, 'C3BAE4955A8C7630', '68378A7D692B7175', 'E419B1B789168FA0', '82DC1F20B760AF63')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9d, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot2)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert
             
    def test_auth_success_Root3_revoked(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, 'CAB8DBF2B5598390', '231C18AA16FC3430', '3E005C246CB901AF', '16C7F873BD021A1F')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9e, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot3)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert
      
    def test_auth_success_Root4_revoked(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, '285ED21C61ED2AB5', '7BF3B4CD41066B0A', 'BE0841E680EE9362', 'D1A0D7D13BA83728')
        #revoke the other Root cert
        status, results = bl.flash_program_once(0x9f, 8, '0000000000000000', 'msb')
        assert status == bootloader.status.kStatus_Success
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SBCACANoCARoot4)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidRootCert  
    
    def test_auth_failed(self,bl):
       #change core to M0+
       common_util.setup_test_environment(bl,'core1',False)
       #write RKTH to target
       write_RKTH(bl, 'E079C6534806E444', 'EDC5A4F829AC3BC9', '6F10E6BE04CF33B4', 'B910781DDEF56552')
       #write signed SB file to 0xa000
       status,result = bl.write_memory(0xa000,SBCACANoCARoot1Failed)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #write M0 demo
       status,result = bl.write_memory(0x09000000, APIAuthPassM0)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #execute demo
       StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
       status, result = bl.execute(PC, 0, StackPointer)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)   
       #get data from 0x20003000
       data = get_data(bl, 0x20003000, 4)
       assert data ==bootloader.status.kStatus_Authentication_SignatureVeriyFailed
          
    def test_auth_pass_key(self,bl):
       #change core to M0+
       common_util.setup_test_environment(bl,'core1',False)
       #write signed SB file to 0xa000
       status,result = bl.write_memory(0xa000,SBCACANoCARoot1)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #write M0 demo
       status,result = bl.write_memory(0x09000000, APIAuthPassKeyM0)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)
       #execute demo
       StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassKeyM0)
       status, result = bl.execute(PC, 0, StackPointer)
       assert status == bootloader.status.kStatus_Success
       time.sleep(1)   
       #get data from 0x20003000
       data = get_data(bl, 0x20003000, 4)
       assert data ==bootloader.status.kStatus_Success
           
           
    def test_auth_profile4096(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, '2371B3DE6ED3622A', '198A735F5D6F2BBC', '7335FADE59ED7213', '03061F552DEDF778')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SB2048CACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIAuthPassM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIAuthPassM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_InvalidKeyLength
            
    def test_auth_profile2048(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, '2371B3DE6ED3622A', '198A735F5D6F2BBC', '7335FADE59ED7213', '03061F552DEDF778')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SB2048CACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIProfile2048M0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIProfile2048M0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Success
           
    def test_auth_rollBack(self,bl):
        #change core to M0+
        common_util.setup_test_environment(bl,'core1',False)
        #write RKTH to target
        write_RKTH(bl, '2371B3DE6ED3622A', '198A735F5D6F2BBC', '7335FADE59ED7213', '03061F552DEDF778')
        #write signed SB file to 0xa000
        status,result = bl.write_memory(0xa000,SB2048CACANoCARoot1)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #write M0 demo
        status,result = bl.write_memory(0x09000000, APIProfileBuildNumber0xacM0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(APIProfileBuildNumber0xacM0)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        time.sleep(1)   
        #get data from 0x20003000
        data = get_data(bl, 0x20003000, 4)
        assert data ==bootloader.status.kStatus_Authentication_DeniedRollBack
       





   
        
        
         
     