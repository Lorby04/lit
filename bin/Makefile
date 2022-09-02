# Build Redis as follows:
#
# - make               -- create non-SGX no-debug-log manifest
# - make SGX=1         -- create SGX no-debug-log manifest
# - make SGX=1 DEBUG=1 -- create SGX debug-log manifest
#
# Any of these invocations clones Redis' git repository and builds Redis in
# default configuration and in the latest-to-date (6.0.5) version.
#
# By default, Redis uses poll/epoll mechanism of Linux. To build Redis with
# select, use `make USE_SELECT=1`. For correct re-builds, always clean up
# Redis source code beforehand via `make distclean`.
#
# Use `make clean` to remove Gramine-generated files and `make distclean` to
# additionally remove the cloned Redis git repository.

################################# CONSTANTS ###################################

# directory with arch-specific libraries, used by Redis
# the below path works for Debian/Ubuntu; for CentOS/RHEL/Fedora, you should
# overwrite this default like this: `ARCH_LIBDIR=/lib64 make`
ARCH_LIBDIR ?= /lib/$(shell $(CC) -dumpmachine)
RM = rm

ifeq ($(DEBUG),1)
GRAMINE_LOG_LEVEL = debug
else
GRAMINE_LOG_LEVEL = error
endif

.PHONY: all
all: lit.manifest
ifeq ($(SGX),1)
all: lit.manifest.sgx lit.sig lit.token
endif

################################ REDIS MANIFEST ###############################

# The template file contains almost all necessary information to run Redis under
# Gramine / Gramine-SGX. We create lit.manifest (to be run under
# non-SGX Gramine) by replacing variables in the template file using the
# "gramine-manifest" script.

lit.manifest: lit.manifest.template
	gramine-manifest \
		-Dlog_level=$(GRAMINE_LOG_LEVEL)  \
		-Darch_libdir=$(ARCH_LIBDIR) \
		$< > $@

# Manifest for Gramine-SGX requires special "gramine-sgx-sign" procedure. This
# procedure measures all Redis trusted files, adds the measurement to the
# resulting manifest.sgx file (among other, less important SGX options) and
# creates lit.sig (SIGSTRUCT object).
#
# Gramine-SGX requires EINITTOKEN and SIGSTRUCT objects (see SGX hardware ABI,
# in particular EINIT instruction). The "gramine-sgx-get-token" script
# generates EINITTOKEN based on a SIGSTRUCT and puts it in .token file. Note
# that filenames must be the same as the manifest name (i.e., "lit").
# EINITTOKEN must be generated on the machine where the application will run,
# not where it was built.

# Make on Ubuntu <= 20.04 doesn't support "Rules with Grouped Targets" (`&:`),
# see the helloworld example for details on this workaround.
lit.sig lit.manifest.sgx: sgx_outputs
	@:

.INTERMEDIATE: sgx_outputs
sgx_outputs: lit.manifest ./lit
	gramine-sgx-sign \
		--manifest lit.manifest \
		--output lit.manifest.sgx

lit.token: lit.sig
	gramine-sgx-get-token --output $@ --sig $<

########################### COPIES OF EXECUTABLES #############################


ifeq ($(SGX),)
GRAMINE = gramine-direct
else
GRAMINE = gramine-sgx
endif
################################## CLEANUP ####################################

.PHONY: clean
clean:
	$(RM) *.token *.sig *.manifest.sgx *.manifest 

