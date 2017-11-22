lib_LTLIBRARIES = libhislip.la

libhislipdir = $(includedir)/hislip
libhislip_HEADERS = include/hislip/client.h \
                    include/hislip/server.h \
                    include/hislip/common.h

libhislip_la_SOURCES = include/hislip/client.h \
                       include/hislip/server.h \
                       include/hislip/common.h \
                       client.c \
                       server.c \
                       tcp.c \
                       tcp.h \
                       session.c \
                       session.h \
                       error.h \
                       message.c \
                       message.h

libhislip_la_CFLAGS = -I$(top_srcdir)/src/include -Wall

libhislip_la_LDFLAGS = -version-info $(LIBHISLIP_LT_VERSION) -init,init -lpthread
