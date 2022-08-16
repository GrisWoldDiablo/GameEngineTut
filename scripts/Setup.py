import os
import subprocess
import platform

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupVulkan import VulkanConfiguration as VulkanRequirements

# Change from Scripts directory to root
os.chdir('./../')

VulkanRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

if platform.system() == "Windows":
    print("\nRunning premake...") 
    subprocess.call([os.path.abspath("./scripts/Win-GenerateVS.bat"), "nopause"])
    print("\nRun bat file located here :")
    print(os.path.abspath("./Hazelnut/SandboxProject/Assets/Scripts/Win-GenerateVS.bat"))


print("\nSetup completed!")
