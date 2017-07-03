import os
import sys
import pytest

filePath = os.path.abspath(os.path.dirname(__file__)).replace('\\', '/')
mainPath = os.path.abspath(os.path.join(filePath, '..')).replace('\\', '/')
sys.path.append(mainPath) # add application path to env

from fsl.bootloader import bltest
from fsl.bootloader import target
from PyUnitTests import bltest_config


@pytest.fixture(scope = 'module')
def tgt():
    # Get the tested CPU type
    targetName = bltest_config.targetName.lower()

    # Get the path of targets directory. (..\targets\cpu)
    targetBaseDir = os.path.join(os.path.dirname(__file__), '..', 'targets', targetName)

    # Check the existence of target directory
    if not os.path.isdir(targetBaseDir):
        raise ValueError("No target directory at path %s" %targetBaseDir)

    # Get the of configuration file for the tested target. (.\targets\$target\bltarget_config.py)
    targetConfigFile = os.path.join(targetBaseDir, 'bltarget_config.py')

    # Check the existence of bltarget_config.py file.
    if not os.path.isfile(targetConfigFile):
        raise RuntimeError("No target configuration file at path %s" %targetConfigFile)

    # Copy the local variables to targetConfig. Note targetConfig is a dictionary.
    targetConfig = locals().copy()
    # Add the file path for bltarget_config.py to targetConfig
    targetConfig['__file__'] = targetConfigFile
    # Add the script file name to targetConfig
    targetConfig['__name__'] = 'bltarget_config'

    # Execute the target config script (bltarget_config.py).
    # Note that, all the variables in bltarget_config.py will store in the dictionary targetConfig
    execfile(targetConfigFile, globals(), targetConfig)

    return target.Target(**targetConfig)

@pytest.fixture(scope = 'function')
def bl(tgt, request):

    BL = bltest.BootloaderDevice(tgt, bltest_config.vectorsDir, bltest_config.peripheral, bltest_config.speed, bltest_config.port,
                                 bltest_config.loadTarget, bltest_config.resetTarget, bltest_config.usePing)

    def closeBootloader():
        BL.close()
    request.addfinalizer(closeBootloader)

    return BL
