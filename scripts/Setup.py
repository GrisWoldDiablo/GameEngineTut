import os
import subprocess
import platform
import sys

from SetupPython import PythonConfiguration as PythonRequirements
from SetupVulkan import VulkanConfiguration as VulkanRequirements   

def main(argv):
    if (len(argv) == 0 or argv[0].lower() != "vsonly"):
        # Make sure everything we need for the setup is installed
        PythonRequirements.Validate() 

        # Change from Scripts directory to root
        os.chdir('./../')   

        VulkanRequirements.Validate()   

        print("\nUpdating submodules...")
        subprocess.call(["git", "submodule", "update", "--init", "--recursive"])  

    else:
        # Change from Scripts directory to root
        os.chdir('./../')

    if platform.system() == "Windows":
        print("\nRunning premakes...") 
        print("")
        subprocess.call(os.path.abspath("./scripts/GenerateHazelProject.bat"), stdin=subprocess.DEVNULL)
        print("")
        subprocess.call(os.path.relpath("./scripts/GenerateSandboxProject.bat"), stdin=subprocess.DEVNULL)   

    print("\nSetup completed!")

if __name__ == "__main__":
    main(sys.argv[1:])