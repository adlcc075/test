############################################################################################
#            ##information about call_no_arg.bin
#call this fucntion: content at 0x20002000 will add 1
#
#            #information about call_with_argc.bin
#call this function: content at 0x20002000 will add the parameter follow command call
#eg. blhost -u -- write-memory 0xa000 call_with_argc.bin
#    blhost -u -- call 0xa001 5
#  the value at 0x20002000 will add 5 after call command executed

import pytest
import sys
import os
import time
import random
filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env
from fsl import bootloader
from fsl.bootloader import commands
from common import common_util

#files for testing call
CallWithoutArgFile = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'call_file' , 'call_no_arg.bin')
CallWithArgFile    = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'call_file' , 'call_with_arg.bin')
ReadFile = os.path.join(os.path.dirname(__file__), 'working', 'vectors' , 'call_file' , 'read_data.bin')

#read one byte and return the content in int type
def read_data(data_file):
     with open(data_file, 'rb') as file:        
         value = file.readline()     
         file.close
     return ord(value)
 
#calculate a invalid address which out of ram to be used for call
def invalid_address(bl):
    RAMStartAddress,RAMEndAddress = common_util.get_memory_start_end_address(bl,'ram')
    print hex(RAMEndAddress)
    print hex(RAMStartAddress)
    return RAMEndAddress

@pytest.fixture(scope = 'function', autouse = True)
def skipif(bl, request):
        common_util.reset_with_check(bl)
        if (commands.kCommandMask_call & common_util.get_available_commands(bl)) == 0:
            pytest.skip('%s does not support call command.' %(bl.target['targetName'])) 
                    
class TestCall:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
         
    def test_call_without_arg(self,bl):  
        #erase region for writing content to flash              
        status, result = bl.flash_erase_region(0xa000, 0x1000)
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        #before excuting call command , read vlaue from 0x20002000
        status, result = bl.read_memory(0x20002000, 1, ReadFile)
        assert status == bootloader.status.kStatus_Success
        DataOriginal = read_data(ReadFile) 
        #write call.bin to 0xa000
        status,result = bl.write_memory(0xa000, CallWithoutArgFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        #call fucntion without argument
        status,result = bl.call(0xa001, 0)
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        #read value from 0x20002000
        status, result = bl.read_memory(0x20002000, 1, ReadFile)
        assert status == bootloader.status.kStatus_Success
        #after excute command call ,the value will add 1
        DataAfterCall = read_data(ReadFile)    
        assert DataAfterCall == DataOriginal + 1
                 
    def test_call_with_arg(self,bl):   
        #erase region for writing content to flash              
        status, result = bl.flash_erase_region(0xa000, 0x1000)
        assert status == bootloader.status.kStatus_Success  
        time.sleep(2)           
        #before excuting call command , read vlaue from 0x20002000
        status, result = bl.read_memory(0x20002000, 1, ReadFile)
        assert status == bootloader.status.kStatus_Success
        DataOriginal = read_data(ReadFile) 
        #write call.bin to 0xa000
        status,result = bl.write_memory(0xa000, CallWithArgFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        #call fucntion without argument
        status,result = bl.call(0xa001, 5)
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        #read value from 0x20002000
        status, result = bl.read_memory(0x20002000, 1, ReadFile)
        assert status == bootloader.status.kStatus_Success
        #after excute command call ,the value will add 5 which defined following call command
        DataAfterCall = read_data(ReadFile)    
        assert DataAfterCall == DataOriginal + 5
         
    def test_call_invalid_address(self,bl):
        RAMEndAddress = invalid_address(bl)
        InvalidAddress = RAMEndAddress + 1
        status, result = bl.call(InvalidAddress, 0)
        assert status == bootloader.status.kStatus_InvalidArgument
     