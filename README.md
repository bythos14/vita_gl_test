# OpenGL ES test using PSM's ScePiglet
Based on [vitaGL sample 6](https://github.com/Rinnegatamante/vitaGL/tree/master/samples/sample6)

Prerequisites:
* DolceSDK (For now, VitaSDK currently lacks SceLibcParam, which Piglet needs for it to use the libc heap.)

* Decrypted libpsm.suprx, libmono_bridge.suprx and libshacccg.suprx (See [FAGDec](https://github.com/CelesteBlue-dev/PSVita-RE-tools/tree/master/FAGDec/build)). These files go into ux0:app/PGLVITA00 after building and installing the vpk.
