##############################################################################
#
#    file                 : Makefile
#    created              : 5 Dec 2007
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
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
SOURCES     = unitmain.cpp \
		unitcarparam.cpp \
		unitcharacteristic.cpp \
		unitclothoid.cpp \
		unitcollision.cpp \
		unitcommon.cpp \
		unitcommondata.cpp \
		unitcubic.cpp \
		unitcubicspline.cpp \
		unitdriver.cpp \
		unitfixcarparam.cpp \
		unitlane.cpp \
		unitlanepoint.cpp \
		unitlinalg.cpp \
		unitlinreg.cpp \
		unitopponent.cpp \
		unitparabel.cpp \
		unitparam.cpp \
		unitpidctrl.cpp \
		unitpit.cpp \
		unitpitparam.cpp \
		unitsection.cpp \
		unitstrategy.cpp \
		unitsysfoo.cpp \
		unitteammanager.cpp \
		unittmpcarparam.cpp \
		unittrack.cpp \
		unitvec2d.cpp \
		unitvec3d.cpp \
		sensors.cpp
SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml 
SHIPSUBDIRS = 0 1 car1-trb1 tracks

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-simplix_V1_10_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-simplix_V1_10_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
