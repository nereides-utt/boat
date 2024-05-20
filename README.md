# Néréides boat's Electrical Systems
## Setting Up the Workspace in Visual Studio Code

After cloning the repository, you need to open the workspace in VS Code that includes all the PlatformIO projects.

### Steps to Set Up Your Workspace






###### 1. Open Visual Studio Code

 - Start by opening VS Code.

###### 2. Open Your Project Folder

- Navigate to `File` > `Open Folder...` and select the root folder of your cloned repository.
- Click on "Open Workspace" in the popup.



###  Add a new folder to the Workspace (PlatformIO Projects)

- Open the Command Palette with `Ctrl+Shift+P` (or `Cmd+Shift+P` on macOS).
- Type `Preferences: Open Workspace Settings (JSON)` and select it.
- Insert the following lines into the JSON settings file:

```json
{
  "folders": [
    {
      "path": "Code/energy"
    },
    {
      "path": "Code/mqtt"
    },
    {
      "path": "Code/cockpit"
    }
  ],
  "settings": {
    "platformio-ide.projectDirs": [
      "Code/energy",
      "Code/mqtt",
      "Code/cockpit"
    ]
  }
}
