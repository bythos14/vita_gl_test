# OpenGL ES test using PSM's ScePiglet

Prerequisites:
* DolceSDK (Since VitaSDK currently lacks SceLibcParam, which Piglet needs for it to use the libc heap.)

* Decrypted libpsm.suprx, libmono_bridge.suprx and libshacccg.suprx (See [FAGDec](https://github.com/CelesteBlue-dev/PSVita-RE-tools/tree/master/FAGDec/build)). These files go into ux0:app/PGLVITA00 after building and installing the vpk.

Import SceLibPsm.elf.gzf into Ghidra to check function offsets.