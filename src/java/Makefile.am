# Java Classes for EAI/SAI/etc.

# Get global variables for Makefile defined by toplevel Makefile.globals
include $(top_srcdir)/Makefile.globals

JAVAROOT=$(top_builddir)/src/java
CLASSPATH_ENV = CLASSPATH=$(JAVAROOT):$(top_srcdir)/src/java:$$CLASSPATH

jarfile = vrml.jar
jardir = $(JAVA_DIR)

$(jarfile): $(noinst_JAVA) manifest.extras classnoinst.stamp
	$(JAR) cmf $(JAVAROOT)/manifest.extras $@ -C $(JAVAROOT) $$(echo $(noinst_JAVA:.java=.class))

jar_DATA = $(jarfile)	

CLEANFILES = $(jarfile) $$(echo $(noinst_JAVA:.java=.class))
EXTRA_DIST = $(noinst_JAVA) java.policy

include Makefile.sources
