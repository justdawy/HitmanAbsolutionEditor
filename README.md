# Hitman Absolution Editor (Localization Fork)

*[Читати українською](README_UK.md)*

This project is a fork of the original **Hitman Absolution Editor**. The main goal of this fork is to add full Ukrainian localization to the game *Hitman: Absolution*.

The original tool only allowed for unpacking and **viewing** game resources, but did not support modifying them. In this version, we have implemented full **editing of fonts and game texts**, along with adding Cyrillic support and tools for mass translation!

## Screenshots

![Text editing and Cyrillic support](assets/screenshot_3.png)
![Global search and resource viewing](assets/screenshot_2.png)
![Importing fonts and other resources](assets/screenshot_1.png)

## What was added and modified:

- **Global Deep Search:** Added the ability to search for any text (e.g., "English" or Cyrillic text) across all 16,000+ game files simultaneously in a background thread, preventing UI freezing.
- **Resource Editing:** Unlike the original, this fork allows you to import and save modified texts and fonts back into the game files (Patch Resource Library Write Back).
- **Cyrillic Support in Editor:** The editor's UI (via ImGui) now correctly renders Ukrainian and Russian characters, both in search results and within the text reading panels.
- **Critical Memory Bug Fixes:** Fixed `Undefined Behavior` issues related to memory allocation for resource arrays, which previously caused application crashes (heap corruption) during mass file scanning.
- **Safe Double-Clicking:** Fixed a bug where double-clicking an unsupported or unknown resource type would crash the editor due to a Null Pointer Exception. The editor now safely ignores these files and logs a warning in the console.
- **Application Icon:** Added a custom Hitman icon to the executable and the window title bar for better OS integration.

## How to Build

1. Clone the repository.
2. Ensure you have **Visual Studio 2022**, **CMake**, and **vcpkg** installed.
3. Run the `build.bat` script in the root folder of the project.
4. After a successful compilation, the ready-to-use `HitmanAbsolutionEditor.exe` will appear in the `build/x64-Release` folder.

## License and Copyright

This project is based on the original Hitman Absolution Editor, which **has no explicit open license**. 
Under GitHub's Terms of Service, by publishing the code, the original author allows other users to view it and create forks within the GitHub platform. 
However, the absence of a license means that **you are not allowed to use this code for commercial purposes or distribute it as your own standalone product**. 
This fork was created exclusively for non-commercial purposes (translating the game into Ukrainian) within the bounds of Fair Use and GitHub's Terms of Service.
