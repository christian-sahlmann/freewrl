# Java Classes for EAI/SAI/etc.

# NOTE - automake-1.11.6 changed how class*.stamp is generated, now need
# to use noinst_JAVA and EXTRA_DIST instead of dist_noinst_JAVA to
# keep behaviour consistent and compatible with both automake-1.11.6 and
# older automake

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
EXTRA_DIST = $(noinst_JAVA) java.policy manifest.extras

include Makefile.sources
