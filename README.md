# AGiliTy

Windows AGiliTy is an interpreter for the old Adventure Game Toolkit (AGT). Although considered outdated and little used these days, AGT was popular in the late 1980s, and a community centered around Compuserve (a commercial bulletin board system popular before the Internet really took off) wrote a large number of games with it. Of these games some are still well worth playing, such as Compuserve and Shades of Gray.

Windows AGiliTy is a port of Robert Masenten's AGiliTy, a reverse engineered interpreter that can play games produced with any of the many versions of AGT.

![AGiliTy playing Shades of Gray](Shades%20of%20Gray.png)

## Building

Download and install Visual Studio 2019 Community edition from https://visualstudio.microsoft.com/. In the installer, under "Workloads", make sure that "Desktop development with C++" is selected, and under "Individual components" that "C++ MFC for latest build tools (x86 & x64)" is selected.

Install git. I use the version of git that is part of Cygwin, a Linux-like environment for Windows, but Git for Windows can be used from a Windows command prompt.

Open the environment that you are using git from (e.g. Cygwin), and switch to the root directory that the build environment will be created under (from here referred to as "\<root>"). Clone this and the other required repositories of mine with git:
```
git clone https://github.com/DavidKinder/Windows-AGiliTy.git Adv/AGiliTy
git clone https://github.com/DavidKinder/Libraries.git Libraries
```

### Third-party libraries

#### libpng

Download the latest version of zlib from https://zlib.net/. Unpack the archive and copy the contents of the top-level directory to "\<root>/Libraries/zlib".

Download the latest version of libpng from http://www.libpng.org/pub/png/libpng.html. Unpack the archive and copy the contents of the top-level directory to "\<root>/Libraries/libpng". Copy the file "\<root>/Libraries/libpng/scripts/pnglibconf.h.prebuilt" to "\<root>/Libraries/libpng/pnglibconf.h".

Open "\<root>/Libraries/libpng/pnglibconf.h" in a text editor, and find and delete all lines that define symbols starting with "PNG_SAVE_", "PNG_SIMPLIFIED_WRITE_" and "PNG_WRITE_".

### Compiling the project

Start Visual Studio, open the solution "\<root>/Adv/AGiliTy/AGiliTy.sln", then build and run the "AGiliTy" project.
