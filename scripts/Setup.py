import os
import subprocess
import platform

from SetupPython import PythonConfiguration as PythonRequirements

#Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupVulkan import VulkanConfiguration as VulkanRequirements

# Change from Scripts directory to root
os.chdir('./../')

VulkanRequirements.Validate()

if platform.system() == "Windows":
    print("\nRunning premake...") 
    subprocess.call([os.path.abspath("./scripts/Win-GenerateVS-2019.bat"), "nopause"])

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

print("\nSetup completed!")
