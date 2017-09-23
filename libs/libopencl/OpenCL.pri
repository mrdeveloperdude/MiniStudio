TARGET = opencl
TEMPLATE = lib

#INCLUDEPATH+="./OpenCL-ICD-Loader/inc"
#LIBS+= -L./OpenCL-ICD-Loader/build/bin
INCLUDEPATH+="/usr/local/include/CL"
LIBS+= -L$$_PRO_FILE_PWD_/OpenCL-ICD-Loader/build/bin
LIBS+= -lOpenCL
PRE_TARGETDEPS += $$_PRO_FILE_PWD_/OpenCL-ICD-Loader/build/bin/libOpenCL.so
