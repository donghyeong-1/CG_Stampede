# Bundled Windows x64 dependencies
- GLFW 3.4: official glfw-3.4.bin.WIN64.zip, lib-vc2022/glfw3.lib (/MD static GLFW).
  https://www.glfw.org/download
- GLEW 2.2.0: official glew-2.2.0-win32.zip, x64 import library and matching DLL.
  https://github.com/nigels-com/glew/releases/tag/glew-2.2.0
- GLM 1.0.3 headers: copied from the existing local GLM installation; no binary library needed.
  https://github.com/g-truc/glm
- Copyright notices and redistribution terms: licenses/ and notices inside each header.
- OpenGL32, User32, Gdi32 and Shell32 come from the standard Windows SDK (not redistributed).
- Visual C++ /MD runtime must be installed. Visual Studio C++ workload installs it; an EXE-only recipient may need the Microsoft Visual C++ 2015-2022 x64 Redistributable.
