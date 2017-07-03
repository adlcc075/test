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

import os
import sys
import random
import inspect
import string
import subprocess
import time
import glob

class JLink():
    def __init__(self, jlinkUsbId, deviceName):
        self._jlinkUsbId = jlinkUsbId
        self._deviceName = deviceName
        # Should add two system variables: SEGGER, MinGW
        self._jlinkGdbServerClExe = os.path.expandvars('%SEGGER%\\JLinkGDBServerCL.exe')
        self._jlinkExe = os.path.expandvars('%SEGGER%\\JLink.exe')
        self._gdbExe = os.path.expandvars('%MinGW%\\bin\\gdb.exe')

        thisFile = inspect.getfile(inspect.currentframe())
        self._currentPath = os.path.abspath(os.path.dirname(thisFile))
        self.recycle_temp_files()

    # Delete the temp files when using the Jlink gdb server and Jlink command line
    def recycle_temp_files(self): 
        for format in ['*.bin', '*.txt', '*.init', '*.jlink']: 
            for tempFile in glob.glob(format):
                try:
                    os.remove(tempFile)
                except IOError as e:
                    print e
                except WindowsError as e:
                    print e

    # @breif Start JLinkGDBServer.
    def start_gdb_server(self):
        gdbConnectionTimes = 5
        for i in xrange(gdbConnectionTimes):
            gdbServerListeningPort = str(random.randint(1000, 4000)) 
            logFile = os.path.join(self._currentPath, 'startGDBServer_%s.txt' %(''.join(random.sample(string.ascii_letters, 10))))
            fileObj = open(logFile, 'w+')
            fileObj.truncate()
            fileObj.close()    
            if self._deviceName == None:
                cmdArgs = '"%s" -select usb=%s -if SWD -port %s -speed auto -singlerun >> %s 2>&1' %(
                            self._jlinkGdbServerClExe,
                            self._jlinkUsbId,
                            gdbServerListeningPort,
                            logFile)
            else:
                cmdArgs = '"%s" -select usb=%s -if SWD -port %s -device %s -speed auto -singlerun >> %s 2>&1' %( 
                            self._jlinkGdbServerClExe,
                            self._jlinkUsbId,
                            gdbServerListeningPort,
                            self._deviceName,
                            logFile)
        
            print '\n-------------Start GDB Server...-------------\n%s' %(cmdArgs)
            p = subprocess.Popen(cmdArgs, bufsize=0, executable=None, stdin=None, stdout=None, stderr=None, shell=True)

            while(p.poll() == None):
                time.sleep(1)
                content = open(logFile,'r').read()
                if 'Waiting for GDB connection...' in content:
                    # It means GDB server started successfully, but the process will not stop running
                    # until the gdb commands are executed sucessfully.
                    # Here the logFile cannot be accessed as it is being used, should delete it when this process ends.
                    print content
                    return gdbServerListeningPort            
            print content
            # The process is finished, the logFile can be deleted.
            os.remove(logFile)
        print("-------------Failed to start the GDB server!-------------")
        assert False
        

     
    # @breif Command execution.    
    def run_script(self, cmd):
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        output = p.communicate()[0]
        print output
            

    # @brief Flash image by JLinkGDBServer. Note that the hexFile and binFile come from the same project.
    def flash_image(self, hexFile, binFile):
        # Get SP and PC from the given bin file. SP: the first four bytes, PC: the second four bytes.
        fileObj = file(binFile, 'r')
        data = fileObj.read(8)
        fileObj.close()
        SP = hex(ord(data[0]) | (ord(data[1]) << 8) | (ord(data[2]) << 16) | (ord(data[3]) << 24))
        PC = hex(ord(data[4]) | (ord(data[5]) << 8) | (ord(data[6]) << 16) | (ord(data[7]) << 24))
        # Start GDB Server
        gdbServerListeningPort = self.start_gdb_server()
        cmdFile = os.path.join(self._currentPath, 'flash_image_%s.init' %(''.join(random.sample(string.ascii_letters, 10))))
        fileObj = file(cmdFile, 'w+')
        fileObj.write(r"set tcp connect-timeout 10" + "\n")
        fileObj.write(r"set remotetimeout 10"  + "\n")
        fileObj.write(r"target remote localhost:%s" %(gdbServerListeningPort) + "\n")
        fileObj.write(r"monitor flash device = %s" %(self._deviceName) + "\n")
        fileObj.write(r"monitor endian little" + "\n")
        fileObj.write(r"monitor speed auto" + "\n") 
        fileObj.write(r"monitor waithalt 5000" + "\n")
        fileObj.write(r'load "%s"' %(hexFile.replace('\\', '/')) + "\n")
        fileObj.write(r"monitor reg SP = %s" %(SP) + "\n")
        fileObj.write(r"monitor reg PC = %s" %(PC) + "\n") 
        
        # For the flashloader_loader and flash-resident bootloader, reset the target here or not
        # are both OK, but on the kl81/kl82, here must add the reset command.
        if (int(PC, 16) & 0x10000000) == 0x0:  # PC points to internal flash
            fileObj.write(r"monitor reset 0" + "\n")
            pass
        elif (int(PC, 16) & 0x10000000) == 0x1:# PC points to SRAM
            pass
        fileObj.write(r"monitor reg SP" + "\n")
        fileObj.write(r"monitor reg PC" + "\n")
        fileObj.write(r"monitor go" + "\n")
        # For kl81/kl82, when download the image to flash, then reset and go, PC will point to ROM, it's a strange issue.
        # So here needs to delay more than 5s so than it will jump to the flash.
        fileObj.write(r"monitor sleep 6000" + "\n")        
        fileObj.write(r"q" + "\n")
        fileObj.close()

        cmd = '%s -x %s' %(self._gdbExe, cmdFile)
        print '\nFlash Image: %s' %('\n' + cmd)
        
        self.run_script(cmd)
     
    # @brief Reset device by JLink GDB server.
    def reset(self):
        # Start GDB Server
        gdbServerListeningPort = self.start_gdb_server()
        cmdFile = os.path.join(self._currentPath, 'reset_%s.init' %(''.join(random.sample(string.ascii_letters, 10))))
        fileObj = file(cmdFile, 'w+')
        fileObj.write(r"set tcp connect-timeout 10" + "\n")
        fileObj.write(r"set remotetimeout 10"  + "\n")
        fileObj.write(r"target remote localhost:%s" %(gdbServerListeningPort) + "\n")
        fileObj.write(r"monitor flash device = %s" %(self._deviceName) + "\n")
        fileObj.write(r"monitor endian little" + "\n")
        fileObj.write(r"monitor speed auto" + "\n")
        fileObj.write(r"monitor reset 0" + "\n")
        fileObj.write(r"q" + "\n")
        fileObj.close()
        
        cmd = '%s -x %s' %(self._gdbExe, cmdFile)
        print '\nReset Target: %s' %('\n' + cmd)
        self.run_script(cmd)
        

    # @brief Dump the binary memory from the specific device by JLink GDB server.
    # Write contents of memory to a raw binary file
    def dump_memory(self, address, bytes):
        dumpMemoryFile = os.path.join(self._currentPath, 'dumpBinary_%s.bin' %(''.join(random.sample(string.ascii_letters, 10))))
        # Start GDB Server
        gdbServerListeningPort = self.start_gdb_server()
        cmdFile = os.path.join(self._currentPath, 'dumpMemory_%s.init' %(''.join(random.sample(string.ascii_letters, 10))))
        fileObj = file(cmdFile, 'w+')
        print >> fileObj, "set tcp connect-timeout 10" 
        print >> fileObj, "set remotetimeout 10"
        print >> fileObj, "target remote localhost:%s" %(gdbServerListeningPort)
        print >> fileObj, "monitor flash device = %s" %(self._deviceName)
        print >> fileObj, "monitor endian little"
        print >> fileObj, "monitor speed auto"
        print >> fileObj, "dump binary memory %s 0x%x 0x%x" %(dumpMemoryFile, address, address + bytes)
        print >> fileObj, "q"
        fileObj.close()
        
        cmd = '%s -x %s' %(self._gdbExe, cmdFile)
        print '\nDump 0x%x bytes data from target address 0x%x:\n%s' %(bytes, address, cmd)
        self.run_script(cmd)
        
        return dumpMemoryFile

    # @brief Restore the binary file to the device memory.
    # Restore the contents of the binary file to target memory.
    def restore_memory(self, memoryAddress, binFile):
        # Start GDB Server
        gdbServerListeningPort = self.start_gdb_server()
        cmdFile = os.path.join(self._currentPath, 'restoreMemory_%s.init' %(''.join(random.sample(string.ascii_letters, 10))))
        fileObj = file(cmdFile, 'w+')
        print >> fileObj, "set tcp connect-timeout 10" 
        print >> fileObj, "set remotetimeout 10"
        print >> fileObj, "target remote localhost:%s" %(gdbServerListeningPort)
        print >> fileObj, "monitor flash device = %s" %(self._deviceName)
        print >> fileObj, "monitor endian little"
        print >> fileObj, "monitor speed auto"
        print >> fileObj, "restore %s binary 0x%x" %(binFile, memoryAddress)
        print >> fileObj, "monitor reset 0"
        print >> fileObj, "q"
        fileObj.close()
        
        cmd = '%s -x %s' %(self._gdbExe, cmdFile)
        print '\nRestore the binary data to the memory address 0x%x:\n%s' %(memoryAddress, cmd)
        self.run_script(cmd)

    # @brief Unlock the Kinetis device by JLink command line.
    def unlock(self):
        cmdFile = os.path.join(self._currentPath, 'unlock_%s.jlink' %(''.join(random.sample(string.ascii_letters, 10))))
        fileObj = file(cmdFile, 'w+')
        fileObj.write('si SWD' + '\n')
        fileObj.write('speed 100' + '\n')
        fileObj.write('unlock kinetis' + '\n')
        fileObj.write('qc' + '\n')
        fileObj.close()
         
        cmd = '"%s" -SelectEmuBySN %s -if SWD -CommanderScript "%s"' %(self._jlinkExe, self._jlinkUsbId, cmdFile)
        print '\nUnlock Device: %s' %('\n' + cmd)
        output = self.run_script(cmd)
     
    # @brief Erase the whole internal flash by JLink command line.
    def erase(self):        
        cmdFile = os.path.join(self._currentPath, 'erase_%s.jlink' %(''.join(random.sample(string.ascii_letters, 10))))
        fileObj = file(cmdFile, 'w+')
        if self._deviceName == None:
            raise ValueError('\nPlease specify the correct device name.')
        fileObj.write('device %s\n' %(self._deviceName))
        fileObj.write('si SWD' + '\n')
        fileObj.write('speed 100' + '\n')
        fileObj.write('rsettype 0' + '\n')
        fileObj.write('erase' + '\n')
        fileObj.write('qc' + '\n')
        fileObj.close()
         
        cmd = '"%s" -SelectEmuBySN %s -if SWD -CommanderScript "%s"' %(self._jlinkExe, self._jlinkUsbId, cmdFile)
        print '\nErase Internal Flash: %s' %('\n' + cmd)
        output = self.run_script(cmd)
 
class ElfTool:
    @staticmethod
    def sparse_elf_info(elfFile):
        readelf_exe = os.path.expandvars('%KDS_WORKBENCH%\\toolchain\\bin\\arm-none-eabi-readelf.exe')
        print 'elfFile: ', elfFile
        cmd = '%s -l %s' %(readelf_exe, elfFile)
        p = subprocess.Popen(cmd, 0, None, None, subprocess.PIPE, subprocess.PIPE, shell=True)
        (sout, serr) = p.communicate()
        # Find the Entry point
        output_arr = sout.split('\n')
        i = 1
        for per_line in output_arr:
            print 'Line %d: %s'%(i, per_line)
            i = i + 1
            if per_line.find('LOAD') != -1:
                vector_start = per_line.split()[3]
                sp = vector_start
                pc = hex(eval(sp) + 4)
                print (sp, pc)
                eturn (sp, pc)
        raise TypeError('\nFile Type Error: please provide the .elf or .out file.')

# debugger = JLink('3094250517','MK66FN2M0XXX18')
# debugger = JLink('1634763797','MKL82Z128XXX7')
# debugger.erase()
# debugger.unlock()
# debugger.reset()
# binFile = debugger.dump_memory(0x20001000,0x4000)
# debugger.restore_memory(0xa000, binFile)   
