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
import string
import shutil
import subprocess
import glob

class Compiler(object):
    def __init__(self):
        # IAR
        self.iarBuildExe = os.path.expandvars('%IAR_WORKBENCH%\\common\\bin\\IarBuild.exe')
        self.ielftoolExe = os.path.expandvars('%IAR_WORKBENCH%\\arm\\bin\\ielftool.exe')
        # KDS
        self.kdsBuildExe = os.path.expandvars('%KDS_WORKBENCH%\\eclipse\\kinetis-design-studio.exe')
        self.armNoneEabiObjcopyExe = os.path.expandvars('%KDS_WORKBENCH%\\toolchain\\bin\\arm-none-eabi-objcopy.exe')
        # KEIL
        self.keilBuildExe = os.path.expandvars('%KEIL_WORKBENCH%\\UV4\\UV4.exe')
        self.fromelfExe = os.path.expandvars('%KEIL_WORKBENCH%\\ARM\\ARMCC\\bin\\fromelf.exe')
    
    # @brief Run the command script.
    def _run_script(self, cmd):
        p = subprocess.Popen(cmd, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        output = p.communicate()[0]
        return output

    # @breif Use ielftool.exe to convert the IAR generated elf file to hex file and bin file.
    def iar_elf2hex2bin(self, elfFile):
        hexFile  = os.path.join(os.path.split(elfFile)[0], os.path.splitext(elfFile)[0] + '.hex')
        binFile  = os.path.join(os.path.split(elfFile)[0], os.path.splitext(elfFile)[0] + '.bin')
        # Convert iar elf file to hex file and bin file
        for fileName in [hexFile, binFile]:
            if fileName == hexFile:
                outputFormat = '--ihex'
            elif fileName == binFile:
                outputFormat = '--bin'
            else:
                pass
            cmd = '"%s" %s %s --verbose %s' %(  self.ielftoolExe,
                                                outputFormat,
                                                elfFile,
                                                fileName)
            # print(cmd)
            output = self._run_script(cmd)
        return (hexFile, binFile)

    # @breif Use arm-none-eabi-objcopy.exe to convert the KDS generated elf file to hex file and bin file.
    def kds_elf2hex2bin(self, elfFile):
        hexFile = os.path.join(os.path.split(elfFile)[0], os.path.splitext(elfFile)[0] + '.hex')
        binFile = os.path.join(os.path.split(elfFile)[0], os.path.splitext(elfFile)[0] + '.bin')
        # Convert kds elf file to hex file and bin file
        for fileName in [hexFile, binFile]:
            if fileName == hexFile:
                outputFormat = 'ihex'
            elif fileName == binFile:
                outputFormat = 'binary'
            else:
                pass
            cmd = '"%s" -O %s "%s" "%s"' %( self.armNoneEabiObjcopyExe,
                                            outputFormat,
                                            elfFile,
                                            fileName)
            # print(cmd)
            output = self._run_script(cmd)
        return (hexFile, binFile)
    
    # @ breif Use fromelf.exe to convert the KEIL generated axfFile file to hex file and bin file.
    def keil_axf2hex2bin(self, axfFile):
        hexFile = os.path.join(os.path.split(axfFile)[0], os.path.splitext(axfFile)[0] + '.hex')
        binFile = os.path.join(os.path.split(axfFile)[0], os.path.splitext(axfFile)[0] + '.bin')        
        # Convert kds elf file to hex file and bin file
        for fileName in [hexFile, binFile]:
            if fileName == hexFile:
                outputFormat = '--i32combined'
            elif fileName == binFile:
                outputFormat = '--bincombined'
            else:
                pass
            cmd = '"%s" %s %s --output %s' %(
                                                self.fromelfExe,
                                                axfFile,
                                                outputFormat,
                                                fileName)
            # print(cmd)
            output = self._run_script(cmd)
        return (hexFile, binFile)

    # @breif Build the iar project to generate elf, hex, bin image files.
    def iar_build(self, ewpFilePath, buildVersion):
        cmd = '"%s" "%s" -build "%s" -log all' %(self.iarBuildExe, ewpFilePath, buildVersion.capitalize())
        print('IAR build for\n%s' %(ewpFilePath))
        output = self._run_script(cmd)
        
        ewpFileFolder = os.path.split(ewpFilePath)[0]
        # The generated image is located at ewpFileFolder\output\buildVersion\xxx.elf
        elfImagePath = os.path.join(ewpFileFolder, 'output', '%s' %buildVersion.capitalize(), '*.elf')
        elfFiles = glob.glob(elfImagePath)
        if len(elfFiles) == 1:
            hexFile, binFile = self.iar_elf2hex2bin(elfFiles[0])
            return (elfFiles[0], hexFile, binFile)
        else:
            print('IAR build fail!')
            raise RuntimeError('IAR build fail!')
    
    # @breif Build the kds project to generate elf, hex, bin image files.
    def kds_build(self, kdsProjectPath, buildVersion):
        kdsFolderPath, kdsProjectName = os.path.split(kdsProjectPath)
        # Create a folder to store the generated data when build
        dataFolder = os.path.join(kdsFolderPath, 'kds_ws_%s' %(''.join(random.sample(string.ascii_letters, 20))))
        os.makedirs(dataFolder)
        
        cmd = '%s --launcher.suppressErrors -nosplash -application "%s" -build "%s/%s" -import "%s" -data "%s"' %(
            self.kdsBuildExe,
            'org.eclipse.cdt.managedbuilder.core.headlessbuild',
            kdsProjectName,
            buildVersion.lower(),
            kdsProjectPath,
            dataFolder
        )
        print('KDS build for\n%s' %(kdsProjectPath))
        output = self._run_script(cmd)
        shutil.rmtree(dataFolder)
        
        elfImagePath = os.path.join(kdsProjectPath, '%s' %buildVersion.lower(), '*.elf')
        elfFiles = glob.glob(elfImagePath)
        if len(elfFiles) == 1:
            hexFile, binFile = self.kds_elf2hex2bin(elfFiles[0])
            return (elfFiles[0], hexFile, binFile)
        else:
            print('KDS build fail!')
            raise RuntimeError('KDS build fail!')
    
    # @brief Build the mdk project to generate elf, hex, bin image files.
    def mdk_build(self, uvprojxFilePath, buildVersion):
        uvprojxFileFolder, uvprojxFileName = os.path.split(uvprojxFilePath)
        uvprojxFileName = uvprojxFileName.split('.')[0] # Get rid of '.uvprojx'
        print uvprojxFileName
        cmd = '%s -r "%s" -j0 -t "%s %s"' %(
            self.keilBuildExe,
            uvprojxFilePath,
            uvprojxFileName,
            buildVersion.capitalize()
        )
        print('MDK build for \n%s' %uvprojxFilePath)
        output = self._run_script(cmd)
        
        elfImagePath = os.path.join(uvprojxFileFolder, 'output', '%s' %buildVersion.capitalize(), '*.elf')
        elfFiles = glob.glob(elfImagePath)
        if len(elfFiles) == 1:
            hexFile, binFile = self.keil_axf2hex2bin(elfFiles[0])
            return (elfFiles[0], hexFile, binFile)
        else:
            print('MDK build fail!')
            raise RuntimeError('KEIL build fail!')    

class KibbleCompiler(Compiler):
    def __init__(self, releasePackage, targetName, projectName, buildVersion):
        # Call the init function of the parent class
        super(KibbleCompiler, self).__init__()
        self.releasePackage = releasePackage
        self.targetName = targetName
        self.projectName = projectName
        self.buildVersion = buildVersion
        
        # Place the generated image to the ..\iar\binaries folder
        self.imageLocation = os.path.join(self.releasePackage, 'targets', '%s' %(self.targetName), 'iar', 'binaries')
    
    # @brief IAR build for the bootloader project in the Kibble release package.
    def kibble_iar_build(self):
        print('====================================================================')
        print('IAR build for %s %s-%s' %(self.targetName, self.projectName, self.buildVersion))
        
        iarFolderPath = os.path.join(self.releasePackage, 'targets', '%s' %(self.targetName), 'iar')
        if self.projectName == 'flashloader_loader':
            # Should first build the flashloader project.
            ewpFilePath = os.path.join(iarFolderPath, 'flashloader', 'flashloader.ewp')
            (elfFile, hexFile, binFile) = self.iar_build(ewpFilePath, self.buildVersion)
        
        ewpFilePath = os.path.join(iarFolderPath, '%s' %(self.projectName), '%s.ewp' %(self.projectName))
        (elfFile, hexFile, binFile) = self.iar_build(ewpFilePath, self.buildVersion)
        
        # Copy the images to one specified location and rename them.
        for image in (elfFile, hexFile, binFile):
            imageCopy = os.path.join(self.imageLocation, 'iar_%s_%s%s' %(self.buildVersion, 
                                                                    self.projectName,
                                                                    os.path.splitext(image)[1]))
            shutil.copy(image, imageCopy)
        return (elfFile, hexFile, binFile)
        
    # @brief KDS build for the bootloader project in the Kibble release package.
    def kibble_kds_build(self):
        print('====================================================================')
        print('KDS build for %s %s-%s' %(self.targetName, self.projectName, self.buildVersion))
        kdsFolderPath = os.path.join(self.releasePackage, 'targets', '%s' %(self.targetName), 'kds')
        
        
        if self.projectName.lower() == 'flashloader_loader':
            # Should first build the flashloader project.
            kdsProjectPath = os.path.join(kdsFolderPath, 'flashloader')
            (elfFile, hexFile, binFile) = self.kds_build(kdsProjectPath, self.buildVersion)

        kdsProjectPath = os.path.join(kdsFolderPath, '%s' %(self.projectName))
        (elfFile, hexFile, binFile) = self.kds_build(kdsProjectPath, self.buildVersion)
        
        # Copy the images to one specified location and rename them.
        for image in (elfFile, hexFile, binFile):
            imageCopy = os.path.join(self.imageLocation, 'kds_%s_%s%s' %(self.buildVersion, 
                                                                    self.projectName,
                                                                    os.path.splitext(image)[1]))
            shutil.copy(image, imageCopy)
        return (elfFile, hexFile, binFile)
    
    # @brief MDK build for the bootloader project in the Kibble release package.
    def kibble_mdk_build(self):    
        print('====================================================================')
        print('MDK build for %s %s-%s' %(self.targetName, self.projectName, self.buildVersion))
        
        mdkFolderPath = os.path.join(self.releasePackage, 'targets', '%s' %(self.targetName), 'mdk')
        if self.projectName.lower() == 'flashloader_loader':
            # Should first build the flashloader project.
            uvprojxFilePath = os.path.join(mdkFolderPath, 'flashloader', 'flashloader.uvprojx')
            (elfFile, hexFile, binFile) = self.mdk_build(uvprojxFilePath, self.buildVersion)
        
        uvprojxFilePath = os.path.join(mdkFolderPath, '%s' %(self.projectName), '%s.uvprojx' %(self.projectName))
        (elfFile, hexFile, binFile) = self.mdk_build(uvprojxFilePath, self.buildVersion)
        
        # Copy the images to one specified location and rename them.
        for image in (elfFile, hexFile, binFile):
            imageCopy = os.path.join(self.imageLocation, 'mdk_%s_%s%s' %(self.buildVersion, 
                                                                         self.projectName,
                                                                         os.path.splitext(image)[1]))
            shutil.copy(image, imageCopy)
        return (elfFile, hexFile, binFile)

    
class AbsKibbleCompiler:
    @staticmethod
    def build(releasePackage, ideName, targetName, projectName, buildVersion):
        # For the ROM validation we do not need to build the project
        if releasePackage == None:
            return None
        else:
            ideName = ideName.lower()
            if ideName == 'iar':
                (elfFile, hexFile, binFile) = KibbleCompiler(releasePackage, targetName, projectName, buildVersion).kibble_iar_build()
            elif ideName == 'kds':
                (elfFile, hexFile, binFile) = KibbleCompiler(releasePackage, targetName, projectName, buildVersion).kibble_kds_build()
            elif ideName == 'mdk':
                (elfFile, hexFile, binFile) = KibbleCompiler(releasePackage, targetName, projectName, buildVersion).kibble_mdk_build()
            else:
                raise RuntimeError('Do not support this ide build!')
             
            return (elfFile, hexFile, binFile)  


# #################################### Testing ################################################            
import jlink_debugger
releasePackage = 'C:\\mqx_cloud_repo\\ksdk_test'
# for file in os.listdir(releasePackage):
#     if os.path.isdir(os.path.join(releasePackage, file)):
#         print file
#  
# for targetName in ['MK64F12']:
#     for projectName in ['flashloader_loader', 'flashloader', 'tower_bootloader', 'freedom_bootloader']:
#         for ideName in ['iar', 'kds', 'mdk']:
#             for buildVersion in ['debug', 'release']:
#                 (elfFile, hexFile, binFile) = AbsKibbleCompiler.build(releasePackage, ideName, targetName, projectName, buildVersion)
#   
#   
# for targetName in ['MK02F12810', 'MKV30F12810']:
#     for projectName in ['flashloader_loader', 'flashloader', 'tower_bootloader']:
#         for ideName in ['kds', 'kds', 'mdk']:
#             for buildVersion in ['debug', 'release']:
#                 (elfFile, hexFile, binFile) = AbsKibbleCompiler.build(releasePackage, ideName, targetName, projectName, buildVersion)           

# target = 'MK02F12810'
# deviceNameName = "MK02FN128XXX10"
# 
# target = 'MKL82Z7'
# deviceNameName = 'MKL82Z128XXX7'
#      
# jlinkUsbId = '1634763797'    
# (elfFile, hexFile, binFile) = AbsKibbleCompiler.build(releasePackage, 
#                                                       'iar', 
#                                                       '%s' %(target), 
#                                                       'freedom_bootloader', 
#                                                       'release')
# jlink = jlink_debugger.JLink(jlinkUsbId, deviceNameName)
# jlink.flash_image(hexFile, binFile)
 
# kdsFolderPath = 'C:\\mqx_cloud_repo\\ksdk_test\\apps\\led_demo\\MK22F51212\\kds'
# iarFolderPath = 'C:\\mqx_cloud_repo\\ksdk_test\\apps\\led_demo\\MK22F51212\\iar'
# mdkFolderPath = 'C:\\mqx_cloud_repo\\ksdk_test\\apps\\led_demo\\MK22F51212\\mdk'
# 
# # KDS build
# led_demo_freedom_0000 = os.path.join(kdsFolderPath, 'led_demo_freedom_0000')
# print Compiler().kds_build(led_demo_freedom_0000, 'Release')
# 
# # IAR build
# led_demo_tower_0000 = os.path.join(iarFolderPath, 'led_demo_tower_0000', 'led_demo_tower_0000.ewp')
# print Compiler().iar_build(led_demo_tower_0000, 'DEBUG')
# 
# mdkProject = 'C:\\mqx_cloud_repo\\ksdk_test\\apps\\led_demo\\MK22F51212\\mdk\\led_demo_freedom_0000\\led_demo_freedom_0000.uvprojx'
# print Compiler().mdk_build(mdkProject, 'release')

