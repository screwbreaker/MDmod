# MDmod PDK
Mega Drive switchless MOD for Padauk microcontroller

![Board](https://github.com/screwbreaker/MDmod/blob/main/PDK/render/MD_Mod_top.png?raw=true)

### Summary
This mod is writted for the PFS173-S14 microcontroller.
But, it must be compatible with other Padauk microncontrollers of the same family. Included the OPT variants.
I've made some test with the PMS152, but not with the final version. Because I ran out of the micro.

If you test this MOD with other micro, please report the results.

### How to compile and program
This MOD must be compiled with [SDCC](http://sdcc.sourceforge.net/).
To program the micro I suggest the FreePDK programmer.
For any other information about the compiler and the programmer, please refer to the [FreePDK](https://free-pdk.github.io/) project.

### Default region
Since this microcontroller family don't have an internal EPROM, the micro always start with the default region.
Is possible to choose the default region by change it in the code before compiling.

### License
The PCBs are under the CERN OHL license.
The firmwares, under the GPL3.

This is an OPEN project.
So, like any other project like this, the hope is that others can improve it an reshare with the community.
Don't go out this basic guideline.

### Disclaimer
The hardware and the software are provided to you 'as is' and without any express or implied warranties whatsoever with respect to its functionality, operability or use, including, without limitation, any implied warranties of merchantability, fitness for a particular purpose or infringement. We expressly disclaim any liability whatsoever for any direct, indirect, consequential, incidental or special damages, including, without limitation, lost revenues, lost profits, losses resulting from business interruption or loss of data, regardless of the form of action or legal theory under which the liability may be asserted, even if advised of the possibility or likelihood of such damages.

### Thanks
- Thanks to the guys of the [Free PDK](https://free-pdk.github.io/) project. They are really smart and awesome!
- The [SDCC](http://sdcc.sourceforge.net/) team.

