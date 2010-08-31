# Java Classes for EAI/SAI/etc.

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

JAVAROOT=$(top_builddir)/src/java
CLASSPATH_ENV = CLASSPATH=$(JAVAROOT):$(top_srcdir)/src/java:$$CLASSPATH

jarfile = vrml.jar
jardir = $(datadir)/java

$(jarfile): $(dist_noinst_JAVA)
	$(JAR) cf $@ -C $(JAVAROOT) $$(echo $(dist_noinst_JAVA:.java=.class))

jar_DATA = $(jarfile)	

CLEANFILES = $(jarfile) $$(echo $(dist_noinst_JAVA:.java=.class))

include Makefile.sources
