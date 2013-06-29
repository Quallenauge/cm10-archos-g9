# Clear out the values
DVP_INC :=
DVP_FEATURES :=
VISION_LIBRARIES :=

DVP_LOCAL_BUILD:=true
DIRECTORIES:=libraries source docs

ifdef DVP_NO_OPTIMIZE
NO_OPTIMIZE:=$(DVP_NO_OPTIMIZE)
endif

include concerto/rules.mak

$(info DVP_FEATURES=$(DVP_FEATURES))
$(info DVP_INC=$(DVP_INC))
$(info VISION_LIBRARIES=$(VISION_LIBRARIES))

