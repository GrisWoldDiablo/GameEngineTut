import os
import sys
import subprocess
from pathlib import Path

import Utils

from io import BytesIO
from urllib.request import urlopen

class VulkanConfiguration:
    requiredVulkanVersion = "1.2.170.0"
    vulkanDirectory = "./Hazel/vendor/VulkanSDK"
    vulkanSDK = os.environ.get("VULKAN_SDK")

    @classmethod
    def Validate(cls):
        if (not cls.CheckVulkanSDK()):
            print("Vulkan SDK not installed correctly.")
            return

        if (not cls.CheckVulkanSDKDebugLibs()):
            print("Vulkan SDK debug libs not found.")
    
    @classmethod
    def CheckVulkanSDK(cls):
        if (cls.vulkanSDK is None):
            print("\nYou don't have the Vulkan SDK installed!")
            cls.__InstallVulkanSDK()
            return False
        else:
            print(f"\nLocated Vulkan SDK ad {cls.vulkanSDK}")

        if (cls.requiredVulkanVersion not in cls.vulkanSDK):
            print(f"You don't have the correct Vulkan SDK version! (Engine requires {cls.requiredVulkanVersion})")
            cls.__InstallVulkanSDK()
            return False
        
        print(f"Correct Vulkan SDK version located at {cls.vulkanSDK}")
        return True
    
    @classmethod
    def __InstallVulkanSDK(cls):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("Would you like to install VulkanSDK {0:s}? [Y/N]: ".format(cls.requiredVulkanVersion))).lower().strip()[:1]
            if reply == 'n':
                print("Process aborted.")
                quit()
            permissionGranted = (reply == 'y')
        vulkanInstallURL = f"https://sdk.lunarg.com/sdk/download/{cls.requiredVulkanVersion}/windows/VulkanSDK-{cls.requiredVulkanVersion}-Installer.exe"
        vulkanPath = f"{cls.vulkanDirectory}/VulkanSDK-{cls.requiredVulkanVersion}-Installer.exe"
        print("Downloading {0:s} to {1:s}".format(vulkanInstallURL, vulkanPath))
        Utils.DownloadFile(vulkanInstallURL, vulkanPath)
        print("Running Vulkan SDK installer...")
        os.startfile(os.path.abspath(vulkanPath))
        print("Re-run this script after installation!")
        quit()

    @classmethod
    def CheckVulkanSDKDebugLibs(cls):
        shadercLib = Path(f"{cls.vulkanSDK}/Lib/shaderc_sharedd.lib")

        VulkanSDKDebugLibsURLlist = [
            f"https://sdk.lunarg.com/sdk/download/{cls.requiredVulkanVersion}/windows/VulkanSDK-{cls.requiredVulkanVersion}-DebugLibs.zip",
            f"https://files.lunarg.com/SDK-{cls.requiredVulkanVersion}/VulkanSDK-{cls.requiredVulkanVersion}-DebugLibs.zip"
        ]

        if not shadercLib.exists():
            print(f"\no Vulkan SDK debug libs found. (Checked {shadercLib})")
            vulkanPath = f"{cls.vulkanSDK}/VulkanSDK-{cls.requiredVulkanVersion}-DebugLibs.zip"
            Utils.DownloadFile(VulkanSDKDebugLibsURLlist, vulkanPath)
            print("Extracting", vulkanPath)
            Utils.UnzipFile(vulkanPath, deleteZipFile=False)
            print(f"Vulkan SDK debug libs installed at {os.path.abspath(cls.vulkanSDK)}")
        else:            
            print(f"Vulkan SDK debug libs located at {os.path.abspath(cls.vulkanSDK)}")
        return True

if __name__ == "__main__":
    VulkanConfiguration.Validate()