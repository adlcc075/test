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


# Command constants.
kCommandMask_flashEraseAll          = 1 << 0
kCommandMask_flashEraseRegion       = 1 << 1
kCommandMask_readMemory             = 1 << 2
kCommandMask_writeMemory            = 1 << 3
kCommandMask_fillMemory             = 1 << 4
kCommandMask_flashSecurityDisable   = 1 << 5
kCommandMask_getProperty            = 1 << 6
kCommandMask_receiveSbFile          = 1 << 7
kCommandMask_execute                = 1 << 8
kCommandMask_call                   = 1 << 9
kCommandMask_reset                  = 1 << 10
kCommandMask_setProperty            = 1 << 11
kCommandMask_flashEraseAllUnsecure  = 1 << 12
kCommandMask_flashProgramOnce       = 1 << 13
kCommandMask_flashReadOnce          = 1 << 14
kCommandMask_flashReadResource      = 1 << 15
kCommandMask_configureQspi          = 1 << 16
kCommandMask_reliableUpdate         = 1 << 17

