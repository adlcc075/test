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

## Utility routine to calculate a status value from a group and code.
def mkstatus(g, c):
    return ((g * 100) + c)

#for execute command
kBlhostError_ReturnedError = -4

# Status groups.
kStatusGroup_Generic = 0
kStatusGroup_FlashDriver = 1
kStatusGroup_I2CDriver = 2
kStatusGroup_SPIDriver = 3
kStatusGroup_QuadSPIDriver = 4
kStatusGroup_OTFADDriver = 5

kStatusGroup_Bootloader = 100
kStatusGroup_SBLoader = 101
kStatusGroup_MemoryInterface = 102
kStatusGroup_PropertyStore = 103
kStatusGroup_AppCrcCheck = 104
kStatusGroup_ReliableUpdate = 106
kStatusGroup_Authentication = 107

# Generic status codes.
kStatus_Success                     = mkstatus(kStatusGroup_Generic, 0)
kStatus_Fail                        = mkstatus(kStatusGroup_Generic, 1)
kStatus_ReadOnly                    = mkstatus(kStatusGroup_Generic, 2)
kStatus_OutOfRange                  = mkstatus(kStatusGroup_Generic, 3)
kStatus_InvalidArgument             = mkstatus(kStatusGroup_Generic, 4)

# Flash driver errors.
kStatus_FlashSizeError              = mkstatus(kStatusGroup_FlashDriver, 0)
kStatus_FlashAlignmentError         = mkstatus(kStatusGroup_FlashDriver, 1)
kStatus_FlashAddressError           = mkstatus(kStatusGroup_FlashDriver, 2)
kStatus_FlashAccessError            = mkstatus(kStatusGroup_FlashDriver, 3)
kStatus_FlashProtectionViolation    = mkstatus(kStatusGroup_FlashDriver, 4)
kStatus_FlashCommandFailure         = mkstatus(kStatusGroup_FlashDriver, 5)
kStatus_FlashUnknownProperty        = mkstatus(kStatusGroup_FlashDriver, 6)

# I2C driver errors.
kStatus_I2C_SlaveTxUnderrun         = mkstatus(kStatusGroup_I2CDriver, 0)
kStatus_I2C_SlaveRxOverrun          = mkstatus(kStatusGroup_I2CDriver, 1)
kStatus_I2C_AribtrationLost         = mkstatus(kStatusGroup_I2CDriver, 2)

# SPI driver errors.
kStatus_SPI_SlaveTxUnderrun         = mkstatus(kStatusGroup_SPIDriver, 0)
kStatus_SPI_SlaveRxOverrun          = mkstatus(kStatusGroup_SPIDriver, 1)

# Bootloader errors.
kStatus_UnknownCommand              = mkstatus(kStatusGroup_Bootloader, 0)
kStatus_SecurityViolation           = mkstatus(kStatusGroup_Bootloader, 1)
kStatus_AbortDataPhase              = mkstatus(kStatusGroup_Bootloader, 2)
kStatus_Ping                        = mkstatus(kStatusGroup_Bootloader, 3)
kStatus_NoResponse                  = mkstatus(kStatusGroup_Bootloader, 4)
kStatus_NoResponseExpected          = mkstatus(kStatusGroup_Bootloader, 5)

# SB loader errors.
kStatusRomLdrSectionOverrun         = mkstatus(kStatusGroup_SBLoader, 0)
kStatusRomLdrSignature              = mkstatus(kStatusGroup_SBLoader, 1)
kStatusRomLdrSectionLength          = mkstatus(kStatusGroup_SBLoader, 2)
kStatusRomLdrUnencryptedOnly        = mkstatus(kStatusGroup_SBLoader, 3)
kStatusRomLdrEOFReached             = mkstatus(kStatusGroup_SBLoader, 4)
kStatusRomLdrChecksum               = mkstatus(kStatusGroup_SBLoader, 5)
kStatusRomLdrCrc32Error             = mkstatus(kStatusGroup_SBLoader, 6)
kStatusRomLdrUnknownCommand         = mkstatus(kStatusGroup_SBLoader, 7)
kStatusRomLdrIdNotFound             = mkstatus(kStatusGroup_SBLoader, 8)
kStatusRomLdrDataUnderrun           = mkstatus(kStatusGroup_SBLoader, 9)
kStatusRomLdrJumpReturned           = mkstatus(kStatusGroup_SBLoader, 10)
kStatusRomLdrCallFailed             = mkstatus(kStatusGroup_SBLoader, 11)
kStatusRomLdrKeyNotFound            = mkstatus(kStatusGroup_SBLoader, 12)
kStatusRomLdrSecureOnly             = mkstatus(kStatusGroup_SBLoader, 13)
kStatusRomLdrSecureOnly             = mkstatus(kStatusGroup_SBLoader, 15)
kStatusRomLdrResetReturned          = mkstatus(kStatusGroup_SBLoader, 14)
kStatusRomLdrRollbackBlocked        = mkstatus(kStatusGroup_SBLoader, 15)
kStatusRomLdrInvalidSectionMacCount = mkstatus(kStatusGroup_SBLoader, 16)
kStatusRomLdrUnexpectedCommand      = mkstatus(kStatusGroup_SBLoader, 17)
kStatusRomLdrBadSBKEK               = mkstatus(kStatusGroup_SBLoader, 18)


# Memory interface errors.
kStatusMemoryRangeInvalid           = mkstatus(kStatusGroup_MemoryInterface, 0)
kStatusMemoryReadFailed             = mkstatus(kStatusGroup_MemoryInterface, 1)
kStatusMemoryWriteFailed            = mkstatus(kStatusGroup_MemoryInterface, 2)
kStatusMemoryCumulativeWrite        = mkstatus(kStatusGroup_MemoryInterface, 3)
# Property store errors.
kStatus_UnknownProperty             = mkstatus(kStatusGroup_PropertyStore, 0)
kStatus_ReadOnlyProperty            = mkstatus(kStatusGroup_PropertyStore, 1)
kStatus_InvalidPropertyValue        = mkstatus(kStatusGroup_PropertyStore, 2)

# Property store errors.
kStatus_AppCrcCheckPassed           = mkstatus(kStatusGroup_AppCrcCheck, 0)
kStatus_AppCrcCheckFailed           = mkstatus(kStatusGroup_AppCrcCheck, 1)
kStatus_AppCrcCheckInactive         = mkstatus(kStatusGroup_AppCrcCheck, 2)
kStatus_AppCrcCheckInvalid          = mkstatus(kStatusGroup_AppCrcCheck, 3)
kStatus_AppCrcCheckOutOfRange       = mkstatus(kStatusGroup_AppCrcCheck, 4)

# QuadSPI driver errors
kStatus_QspiFlashSizeError          = mkstatus(kStatusGroup_QuadSPIDriver, 0)
kStatus_QspiFlashAlignmentError     = mkstatus(kStatusGroup_QuadSPIDriver, 1)
kStatus_QspiFlashAddressError       = mkstatus(kStatusGroup_QuadSPIDriver, 2)
kStatus_QspiFlashCommandFailure     = mkstatus(kStatusGroup_QuadSPIDriver, 3)
kStatus_QspiFlashUnknownProperty    = mkstatus(kStatusGroup_QuadSPIDriver, 4)
kStatus_QspiNotConfigured           = mkstatus(kStatusGroup_QuadSPIDriver, 5)
kStatus_QspiCommandNotSupported     = mkstatus(kStatusGroup_QuadSPIDriver, 6)

kStatus_OtfadSecurityViolation      = mkstatus(kStatusGroup_OTFADDriver, 0)
kStatus_OtfadLogicallyDisabled      = mkstatus(kStatusGroup_OTFADDriver, 1)
kStatus_OtfadInvalidKey             = mkstatus(kStatusGroup_OTFADDriver, 2)

#reliable update
kStatus_ReliableUpdateSuccess                     = mkstatus(kStatusGroup_ReliableUpdate, 0)
kStatus_ReliableUpdateFail                        = mkstatus(kStatusGroup_ReliableUpdate, 1)
kStatus_ReliableUpdateInactive                    = mkstatus(kStatusGroup_ReliableUpdate, 2)
kStatus_ReliableUpdateBackupApplicationInvalid    = mkstatus(kStatusGroup_ReliableUpdate, 3)
kStatus_ReliableUpdateStillInMainApplication      = mkstatus(kStatusGroup_ReliableUpdate, 4)
kStatus_ReliableUpdateSwapSystemNotReady          = mkstatus(kStatusGroup_ReliableUpdate, 5)
kStatus_ReliableUpdateBackupBootloaderNotReady    = mkstatus(kStatusGroup_ReliableUpdate, 6)
kStatus_ReliableUpdateSwapIndicatorAddressInvalid = mkstatus(kStatusGroup_ReliableUpdate, 7)

#authentication
kStatus_Authentication_FetalError                 = mkstatus(kStatusGroup_Authentication, 0)
kStatus_Authentication_NotInitialized             = mkstatus(kStatusGroup_Authentication, 1)
kStatus_Authentication_DataUnderrun               = mkstatus(kStatusGroup_Authentication, 2)
kStatus_Authentication_InvalidFormat              = mkstatus(kStatusGroup_Authentication, 3)
kStatus_Authentication_DeniedRollBack             = mkstatus(kStatusGroup_Authentication, 4)
kStatus_Authentication_OverMaxCertificateSize     = mkstatus(kStatusGroup_Authentication, 5)
kStatus_Authentication_InvalidCertFormat          = mkstatus(kStatusGroup_Authentication, 6)
kStatus_Authentication_InvalidKeyLength           = mkstatus(kStatusGroup_Authentication, 7)
kStatus_Authentication_CertVerifyFailed           = mkstatus(kStatusGroup_Authentication, 8)
kStatus_Authentication_AllocateFailed             = mkstatus(kStatusGroup_Authentication, 9)
kStatus_Authentication_InvalidRootKeyTableHash    = mkstatus(kStatusGroup_Authentication, 10)
kStatus_Authentication_InvalidRootCert            = mkstatus(kStatusGroup_Authentication, 11)
kStatus_Authentication_SignatureVeriyFailed       = mkstatus(kStatusGroup_Authentication, 12)
kStatus_Authentication_Finalized                  = mkstatus(kStatusGroup_Authentication, 13)
