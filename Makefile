.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk


LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)
LIB_NAME = parsepcf


OBJ =\
	libparsepcf_destroy_preparsed_font.o\
	libparsepcf_get_accelerators.o\
	libparsepcf_get_bitmap_offsets.o\
	libparsepcf_get_bitmaps.o\
	libparsepcf_get_encoding.o\
	libparsepcf_get_glyph_indices.o\
	libparsepcf_get_glyph_name_subtable.o\
	libparsepcf_get_glyph_names.o\
	libparsepcf_get_metrics.o\
	libparsepcf_get_metrics_count.o\
	libparsepcf_get_properties.o\
	libparsepcf_get_property_subtable.o\
	libparsepcf_get_swidth_count.o\
	libparsepcf_get_swidths.o\
	libparsepcf_get_table_count.o\
	libparsepcf_get_tables.o\
	libparsepcf_parse_int16_from_unsigned__.o\
	libparsepcf_parse_int32_from_unsigned__.o\
	libparsepcf_parse_lsb_uint16__.o\
	libparsepcf_parse_lsb_uint32__.o\
	libparsepcf_parse_msb_uint16__.o\
	libparsepcf_parse_msb_uint32__.o\
	libparsepcf_preparse_font.o

HDR =\
	libparsepcf.h\
	common.h

LOBJ = $(OBJ:.o=.lo)
SRC = $(OBJ:.o=.c)


all: libparsepcf.a libparsepcf.$(LIBEXT) demo
$(OBJ): $(HDR)
$(LOBJ): $(HDR)

demo: demo.o libparsepcf.a
	$(CC) -o $@ demo.o libparsepcf.a $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

libparsepcf.a: $(OBJ)
	@rm -f -- $@
	$(AR) rc $@ $(OBJ)

libparsepcf.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) -o $@ $(LOBJ) $(LDFLAGS)

install: libparsepcf.a libparsepcf.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	cp -- libparsepcf.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libparsepcf.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libparsepcf.$(LIBMINOREXT)"
	$(FIX_INSTALL_NAME) "$(DESTDIR)$(PREFIX)/lib/libparsepcf.$(LIBMINOREXT)"
	ln -sf -- libparsepcf.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libparsepcf.$(LIBMAJOREXT)"
	ln -sf -- libparsepcf.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libparsepcf.$(LIBEXT)"
	cp -- libparsepcf.h "$(DESTDIR)$(PREFIX)/include/"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libparsepcf.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libparsepcf.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libparsepcf.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libparsepcf.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libparsepcf.h"

clean:
	-rm -f -- *.o *.a *.lo *.su *.so *.so.* *.dll *.dylib
	-rm -f -- *.gch *.gcov *.gcno *.gcda *.$(LIBEXT) demo

.SUFFIXES:
.SUFFIXES: .lo .o .c

.PHONY: all install uninstall clean
