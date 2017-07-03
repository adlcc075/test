#! /usr/bin/env python

# Copyright (c) 2013 Freescale Semiconductor, Inc.
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

# Property constants.
kPropertyTag_CurrentVersion             = 1
kPropertyTag_AvailablePeripherals       = 2
kPropertyTag_FlashStartAddress          = 3
kPropertyTag_FlashSizeInBytes           = 4
kPropertyTag_FlashSectorSize            = 5
kPropertyTag_FlashBlockCount            = 6
kPropertyTag_AvailableCommands          = 7
kPropertyTag_CrcCheckStatus             = 8

kPropertyTag_VerifyWrites               = 10
kPropertyTag_MaxPacketSize              = 11
kPropertyTag_ReservedRegions            = 12
kPropertyTag_ValidateRegions            = 13
kPropertyTag_RAMStartAddress            = 14
kPropertyTag_RAMSizeInBytes             = 15
kPropertyTag_SystemDeviceIdent          = 16
kPropertyTag_FlashSecurityState         = 17
kPropertyTag_UniqueDeviceIdent          = 18
kPropertyTag_FacSupportFlag             = 19
kPropertyTag_FacSegmentSize             = 20
kPropertyTag_FacSegmentCount            = 21
kPropertyTag_FlashReadMargin            = 22
kPropertyTag_QspiInitStatus             = 23
kPropertyTag_TargetVersion              = 24
kPropertyTag_ExternalMemoryAttrubutes   = 25
kPropertyTag_ReliableUpdate             = 26
