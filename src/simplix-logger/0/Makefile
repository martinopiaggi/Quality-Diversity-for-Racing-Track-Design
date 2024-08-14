##############################################################################
#
#    file                 : Makefile
#    created              : 05 Dec 2007
#    copyright            : (C) 2007 Wolf-Dieter Beelitz
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = simplix_V1_10
SHIPDIR     = drivers/${ROBOT}/0
SHIP        = car1-trb1.rgb
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_V1_10_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_V1_10_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

