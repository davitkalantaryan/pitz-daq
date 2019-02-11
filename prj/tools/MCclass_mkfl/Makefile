#
# File:         Makefile 
#
#		shm_lib
#
# $Date: 2010/01/11 14:35:10 $
# $Source: /afs/ifh.de/group/pitz/doocs/develop/levonh/source/MCclasslib/Makefile,v $
# $Revision: 1.16 $
# $State: Exp $
#
#
# Author:       Gerhard Grygiel <grygiel@desy.de>
#               Copyright 1995
#
#               This program is free software; you can redistribute it
#               and/or  modify it under  the terms of  the GNU General
#               Public  License as  published  by  the  Free  Software
#               Foundation;  either  version 2 of the License, or  (at
#               your option) any later version.
#
#
#

DOOCSROOT = ../..

# to define DOOCSROOT as an absolute path
include $(DOOCSROOT)/$(DOOCSARCH)/DEFINEDOOCSROOT

# to define LIBNO
include ./LIBNO
SVR_VER_CVS=1.0.0

OBJDIR = $(DOOCSROOT)/$(DOOCSARCH)/obj/$(PKGDIR)

SRCDIR = $(DOOCSROOT)/source/$(PKGDIR)


CPPFLAGS    += -DDEBUG -D_SYSCALL32 
#CPPFLAGS    += -xsb 
#CPPFLAGS    += -fast 

#CCFLAGS += -xO2
##CPPFLAGS += -I$(DOOCSROOT)/source/serverlib
##LDFLAGS += -L$(DOOCSROOT)/$(DOOCSARCH)/obj/serverlib

# DAQ XML parcer library
#DAQIBI		= /home/petros/doocs/source/daq/bm
#OBJPRS		= /home/arthura/doocs/solaris2/obj/XML_tools
#SRCPRS		= /home/arthura/doocs/source/daq/XML_tools
#CPPFLAGS       += -I/home/arthura/doocs/source/senderlib
#LDFLAGS        += -L/home/arthura/doocs/$(DOOCSARCH)/obj/senderlib
#OBJFSM         = /home/arthura/doocs/solaris2/obj/daq/fsm
#SRCFSM         = /home/arthura/doocs/source/daq/fsm
#CPPFLAGS	+= -I$(SRCFSM) -I$(SRCPRS) -I$(DAQIBI)
#LDFLAGS 	+= -L$(OBJFSM) -L$(OBJPRS)

# to define the arch dependend things
include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG
# include $(DOOCSROOT)/$(DOOCSARCH)/CONFIG.loc
LIBRARYOBJ = \
        $(OBJDIR)/MCclass.o

LIBRARYHFILES = MCclass.h

PSFILES =

ALLLIBS = $(OBJDIR)/libMCclass.so.$(LIBNO) 
	
all: $(ALLLIBS) $(PSFILES)

$(OBJDIR)/.depend depend:
		@if [ ! -f $(OBJDIR) ] ; then \
		  echo ---------- create dir $(OBJDIR) --------------; \
		  mkdir -p $(OBJDIR) ; \
		fi
		for i in $(SRCDIR)/*.cc ;do $(CCDEP) $$i ;done > $(OBJDIR)/.depend_temp
		cat $(OBJDIR)/.depend_temp | sed -e "/:/s/^/\$$\(OBJDIR\)\//g" > $(OBJDIR)/.depend
		chmod g+w $(OBJDIR)/.depend*

include $(OBJDIR)/.depend

$(OBJDIR)/libMCclass.a:    $(LIBRARYOBJ)	
		$(LINK.a) $(LIBRARYOBJ)
		@-chmod g+w $(OBJDIR)/libMCclass.a 
		@echo "---------------- $(OBJDIR)/libMCclass.a done---------------"

$(OBJDIR)/libMCclass.so.$(LIBNO): $(LIBRARYOBJ)
		$(LINK.so) $(LIBRARYOBJ)
		@-chmod g+w $(OBJDIR)/libMCclass.s*.$(LIBNO)
		@rm -f $(OBJDIR)/libMCclass.so
		@ln -s $(OBJDIR)/libMCclass.so.$(LIBNO) $(OBJDIR)/libMCclass.so
		@echo "------ $(OBJDIR)/libMCclass.so.$(LIBNO) done---------------"

localinstall:  $(ALLLIBS)
		# install H files
		@if [ ! -d $(DOOCSINC) ] ; then \
		 echo "- INSTALL: create dir $(DOOCSINC) " ; \
		 mkdir -p $(DOOCSINC) ; \
		 chmod -f g+w $(DOOCSINC) ; \
		fi
		@for i in $(LIBRARYHFILES); do \
		 if [ -f $(DOOCSINC)/`basename $$i` ] ; then \
		    echo "- INSTALL: move $(DOOCSINC)/`basename $$i` to \n  $(DOOCSINC)/`basename $$i`.OLD" ; \
		    mv $(DOOCSINC)/`basename $$i` $(DOOCSINC)/`basename $$i`.OLD ; \
		 fi ; \
		 echo "- INSTALL: $$i in \n  $(DOOCSINC)/`basename $$i` " ; \
		 cp $$i $(DOOCSINC) ;\
		 chmod -f g+w $(DOOCSINC)/`basename $$i` ; \
		done
		# install in the main arch lib dir
		@if [ ! -d $(DOOCSLIBS) ] ; then \
		 echo "- INSTALL: create dir $(DOOCSLIBS) " ; \
		 mkdir -p $(DOOCSLIBS) ; \
		 chmod -f g+w $(DOOCSLIBS) ; \
		fi
		@for p in $(ALLLIBS); do \
		 echo "- INSTALL: $$p in \n  $(DOOCSLIBS)/`basename $$p` " ; \
		 cp $$p $(DOOCSLIBS) ; \
		 chmod -f g+w $(DOOCSLIBS)/`basename $$p` ; \
		 if [ "XSO" = X`echo $$p | awk '/\.so/ {print "SO"} '` ] ; then \
			SONAME=`echo $$p | sed -e "s/\.so.\{0,15\}/.so/g"` ; \
			echo "- INSTALL: ln -s $(DOOCSLIBS)/`basename $$p` $(DOOCSLIBS)/`basename $$SONAME`" ; \
			rm -f $(DOOCSLIBS)/`basename $$SONAME` ; \
			ln -s $(DOOCSLIBS)/`basename $$p` $(DOOCSLIBS)/`basename $$SONAME` ; \
			SAFILE=`echo $$p  | sed -e "s/\.so\./\.sa\./g"` ; \
			if [ -f $$SAFILE ] ; then \
			   echo "- INSTALL: $$SAFILE in \n  $(DOOCSLIBS)/`basename $$SAFILE` " ; \
			   cp $$SAFILE $(DOOCSLIBS) ; \
			   chmod -f g+w $(DOOCSLIBS)/`basename $$SAFILE` ; \
			fi ; \
		 fi ; \
		done

developmentinstall:  $(ALLLIBS)
		# install in all INSTLIBDIRSDEV
		-@for i in $(INSTLIBDIRSDEV) ; do \
		   if [ ! -d $$i/include ] ; then \
		      echo "- INSTALL: create dir $$i/include " ; \
		      mkdir -p $$i/include ; \
		      chmod -f g+w $$i/include ; \
		   fi ; \
		   for k in $(LIBRARYHFILES); do \
		    if [ -f $$i/include/`basename $$k` ] ; then \
		       echo "- INSTALL: move $$i/include/`basename $$k` to \n  $$i/include/`basename $$k`.OLD" ; \
		       mv $$i/include/`basename $$k` $$i/include/`basename $$k`.OLD ; \
		    fi ; \
		    echo "- INSTALL: $$k in \n  $$i/include/`basename $$k` " ; \
		    cp $$k $$i/include ;\
		    chmod -f g+w $$i/include/`basename $$k` ; \
		   done ; \
		  for p in $(ALLLIBS); do \
		   if [ "XSO" = X`echo $$p | awk '/\.so/ {print "SO"} '` ] ; then \
			echo "- INSTALL: $$p in \n  $$i/`basename $$p` " ; \
			cp $$p $$i ; \
			chmod -f g+w $$i/`basename $$p` ; \
			SONAME=`echo $$p | sed -e "s/\.so.\{0,15\}/.so/g"` ; \
			echo "- INSTALL: ln -s $$i/`basename $$p` $$i/`basename $$SONAME`" ; \
			rm -f $$i/`basename $$SONAME` ; \
			ln -s $$i/`basename $$p` $$i/`basename $$SONAME` ; \
			SAFILE=`echo $$p  | sed -e "s/\.so\./\.sa\./g"` ; \
			if [ -f $$SAFILE ] ; then \
			   echo "- INSTALL: $$SAFILE in \n  $$i/`basename $$SAFILE` " ; \
			   cp $$SAFILE $$i ; \
			   chmod -f g+w $$i/`basename $$SAFILE` ; \
			fi ; \
		   fi ; \
		 done \
		done
		echo "Running Doxygen "
		doxygen MCclass.doxygen 

remoteinstall:  $(ALLLIBS)
		# install in all ALLLIBS
		@for i in $(INSTLIBDIRS); do \
		 for p in $(ALLLIBS); do \
		  if [ ! -d $$i ] ; then \
		     echo "- INSTALL: create dir $$i " ; \
		     mkdir -p $$i ; \
		     chmod -f g+w $$i ; \
		  fi ; \
		  if [ "XSO" = X`echo $$p | awk '/\.so/ {print "SO"} '` ] ; then \
			echo "- INSTALL: $$p in \n  $$i/`basename $$p` " ; \
			cp $$p $$i ; \
			chmod -f g+w $$i/`basename $$p` ; \
		  fi ; \
		 done \
		done


install:$(ALLLIBS)
	-cvs commit -m "new $(PKGDIR) $(LIBNO)"
	-cvs tag LIBNO_`echo $(LIBNO) | sed -e "s/\./_/g" `
	cd $(DOOCSROOT)/$(DOOCSARCH) ; \
	cvs commit -m "new $(PKGDIR) $(LIBNO)" CONFIG ; \
	cvs tag LIBNO_`echo $(LIBNO) | sed -e "s/\./_/g" ` CONFIG ; \
	cd $(DOOCSVERSIONS) ; \
	cvs checkout $(DOOCSARCH)/CONFIG ; \
	rm -f source/$(PKGDIR) ; \
	if test ! -d source/$(PKGDIR)-$(LIBNO); then \
		cvs checkout source/$(PKGDIR) ; \
		mv source/$(PKGDIR) source/$(PKGDIR)-$(LIBNO) ; \
		rm -f $(DOOCSVERSIONS)/source/$(PKGDIR) ; \
		cd source ; \
		ln -s $(PKGDIR)-$(LIBNO) $(PKGDIR) ; \
		cd .. ; \
	else \
		chmod -R ug+w source/$(PKGDIR)-$(LIBNO) ; \
		cvs update source/$(PKGDIR)-$(LIBNO) ; \
	fi ; \
	cd source/$(PKGDIR)-$(LIBNO) ; \
	make DOOCSARCH=$(DOOCSARCH) all ; \
	make DOOCSARCH=$(DOOCSARCH) developmentinstall ; \
	cd $(DOOCSVERSIONS) ; \
	echo " installed in: $(INSTLIBDIRSDEV) "
	echo " ! NO LOCAL INSTALL ! "

clean:
	rm -f $(LIBRARYOBJ) $(OBJDIR)/libMCclass.* $(OBJDIR)/.depend

update:
	cvs update

commit:
	cvs commit

import:
	cvs import -b $(SVR_VER_CVS) source/libMCclass doocs start
