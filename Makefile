TOP=$(HOME)/linwx
BASE_OMNI_TREE=$(HOME)
INSTALL=$(HOME)/bin/scripts/install-sh

# you must modify follows for compile you program correctly.

VPATH=linwx/test
CURRENT=linwx/test

# modify end.

include $(TOP)/mk/beforeauto.mk
include ./dir.mk
include $(TOP)/mk/afterauto.mk
