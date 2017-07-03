import pytest
import sys
import os
import time
import array
filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env
from fsl import bootloader
from fsl.bootloader import commands
from common import common_util

def generate_demo(bl):
    elfFile, hexFile, binFile = common_util.get_led_demo_path(bl)
    return binFile

def get_invalid_pc(bl):
    RAMStartAddress,RAMEndAddress = common_util.get_memory_start_end_address(bl,'ram')
    print hex(RAMEndAddress)
    print hex(RAMStartAddress)
    return RAMEndAddress

def get_valid_stack_pointer(bl):
    RAMStartAddress,RAMEndAddress = common_util.get_memory_start_end_address(bl,'ram')
    print hex(RAMStartAddress - 8)
    return RAMEndAddress - 8

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
    
@pytest.fixture(scope = 'function', autouse = True)
def skipif(bl, request):
        common_util.reset_with_check(bl)
        if (commands.kCommandMask_execute & common_util.get_available_commands(bl)) == 0:
            pytest.skip('%s does not support call command.' %(bl.target['targetName'])) 
            
class TestExecute:
    @pytest.fixture(autouse=True)
    def setup(self, request, bl):
        common_util.reset_with_check(bl)
        bl.flash_erase_region(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, 0x1000)
        time.sleep(2)
         
    def test_execute_without_arg(self,bl):
        #write demo to app vector table
        DemoFile = generate_demo(bl)
        status,result = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, DemoFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(DemoFile)
        status, result = bl.execute(PC, 0, StackPointer)
        assert status == bootloader.status.kStatus_Success
        #check whether had jump to app
        status,result = bl.get_property(1)
        assert status == bootloader.status.kBlhostError_ReturnedError
        time.sleep(2)
          
    def test_execute_with_arg(self,bl):
        #write demo to app vector table
        DemoFile = generate_demo(bl)
        status,result = bl.write_memory(bl.target.BL_APP_VECTOR_TABLE_ADDRESS, DemoFile)
        assert status == bootloader.status.kStatus_Success
        time.sleep(2)
        #execute demo
        StackPointer, PC = get_PC_stack_address_from_demo(DemoFile)
        status, result = bl.execute(PC, 5, StackPointer)
        assert status == bootloader.status.kStatus_Success
        #check whether had jump to app
        status,result = bl.get_property(1)
        assert status == bootloader.status.kBlhostError_ReturnedError
        time.sleep(2)
         
    def test_execute_with_ignore_arg(self,bl):
        
        PC           = get_invalid_pc(bl)
        StackPointer = get_valid_stack_pointer(bl)
        status, result = bl.execute(PC, 5, StackPointer)
        assert status == bootloader.status.kStatus_InvalidArgument



        
            
            

     
      

