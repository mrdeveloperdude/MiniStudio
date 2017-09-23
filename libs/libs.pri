
#INCLUDEPATH+="./OpenCL-ICD-Loader/inc"
#LIBS+= -L./OpenCL-ICD-Loader/build/bin


libs = \
#	drumstick \
	opencl \
	style \
	alsa \
	util \


for(lib, libs){
	INC="$$PWD/lib$$lib"
	# HACK
	OUT=$$clean_path("$$OUT_PWD/../libs/lib$$lib")
	message("LIB INC:" $$INC $$OUT)
	INCLUDEPATH+=$$INC
	LIBS+= -L$$OUT
	LIBS+= -l$$lib
	PRE_TARGETDEPS += "$$OUT/$$lib.a"

}

