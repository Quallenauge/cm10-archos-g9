if [ -z "${ANDROID_PRODUCT_OUT}" ]; then exit; fi;
if [ -z "${DVP_ROOT}" ]; then exit; fi;

VISIONAPP=com.ti.dvp.VisionApp.VisionAppActivity
DEST=${DVP_ROOT}/samples/${VISIONAPP}/jni/lib/armeabi

if [ ! -d "${DEST}" ]; then mkdir -p ${DEST}; fi

SLIBS="imgfilter vcam sosal cthreaded"
DLIBS="dvp dvp_kgm_cpu dvp_kgm_dsp dvp_kgm_simcop OMX_Core utils cutils binder camera_client ui gui"

for slib in ${SLIBS}; do 
	cp ${ANDROID_PRODUCT_OUT}/obj/STATIC_LIBRARIES/lib${slib}_intermediates/lib${slib}.a ${DEST}
done

for dlib in ${DLIBS}; do 
	cp ${ANDROID_PRODUCT_OUT}/symbols/system/lib/lib${dlib}.so ${DEST}
done

