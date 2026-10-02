# Include macros from the m4 directory
ACLOCAL_AMFLAGS = -I m4

all: highway prism
	
highway:
	$(BAZEL) build @hwy

# The installed shared libraries come from //lib, which links each one against
# the libraries it needs (libprism-config.so, libhwy.so). The .so files Bazel
# builds in bazel-bin/src record no such dependencies.
prism:
	$(BAZEL) build src:prism-all lib:shared-libraries

bazel-bin/prism-dynamic.ll:
	$(BAZEL) build :prism_ir

bazel-bin/prism-static.ll:
	$(BAZEL) build :prism_ir

include_HEADERS= \
	bazel-bin/prism-dynamic.ll \
	bazel-bin/prism-static.ll \
	src/prism_api.h \
	src/prism_array.h \
	src/sr_array.h \
	src/ud_array.h \
	src/sr_scalar.h \
	src/ud_scalar.h \
	src/prism.h

EXTRA_DIST = \
	bazel-bin/lib/libhwy.so \
	bazel-bin/lib/libprism-config.so \
	bazel-bin/lib/libprism-dynamic.so \
	bazel-bin/lib/libprism-static.so \
	bazel-bin/lib/libprism-dynamic-dbg.so \
	bazel-bin/lib/libprism-static-dbg.so

install-exec-hook:
	@echo "Installing prism library"
	$(MKDIR_P) $(DESTDIR)$(libdir)
	install -m 0755 bazel-bin/lib/libhwy.so $(DESTDIR)$(libdir)
	install -m 0755 bazel-bin/lib/libprism-config.so $(DESTDIR)$(libdir)
	install -m 0755 bazel-bin/lib/libprism-dynamic-dbg.so $(DESTDIR)$(libdir)
	install -m 0755 bazel-bin/lib/libprism-dynamic.so $(DESTDIR)$(libdir)
	install -m 0755 bazel-bin/lib/libprism-static-dbg.so $(DESTDIR)$(libdir)
	install -m 0755 bazel-bin/lib/libprism-static.so $(DESTDIR)$(libdir)

clean:
	$(BAZEL) clean