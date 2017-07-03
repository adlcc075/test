import os
import shutil
import time

# Give the path that blhost.exe tool locates at.
# blhostPath = "D:\\Kibble_test\\Kibble K60_2M\\k60-2m_flashloader_1.0rc2\\app"
blhostPath = "D:\\ROM_test\L5KS\\RC_packages\\L5K_ROM_release_1.0rc3"
# blhostPath = "D:\\Kibble_test\\Kibble1.1\\FSL_Kinetis_Bootloader_1_1_0_rc7\\bin\\win"


# Get the location of the blhost.exe tool.
srcFile = os.path.join(blhostPath, 'blhost.exe')

# Should copy the blhost.exe to the working directory.
destDir = os.path.join(os.path.dirname(__file__), '..', 'PyUnitTests', 'working')


# Delete the old version of the blhost.exe tool if it exists.
blhost = os.path.join(destDir, "blhost.exe")
if os.path.exists(blhost):
    print 'Deleting the old version of the blhost.exe --> ',
    os.remove(blhost)
else:
    print 'Not find the blhost.exe --> ',

# Copy the blhost.exe to the working directory.
print 'copying blhost.exe tool --> ',
time.sleep(3)
shutil.copy(srcFile, destDir)
print 'Done'
