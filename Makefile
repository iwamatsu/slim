# Makefile for slim - generic Linux
#
# Edit the following section to adjust the options
# to fit into your operating system / distribution
#######################################################
CXX=g++
CC=gcc
CFLAGS=-Wall -I. -I/usr/include/freetype2 -I/usr/include/freetype2/config -I/usr/include
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lXft -lX11 -lfreetype -lXrender -lfontconfig -lpng -lz -lm -lcrypt -lXmu -lpng -ljpeg -lrt
CUSTOM=-DHAVE_SHADOW
ifdef USE_PAM
LDFLAGS+= -lpam
CUSTOM+= -DUSE_PAM
endif
PREFIX=/usr
CFGDIR=/etc
MANDIR=/usr/man
DESTDIR=
#######################################################

NAME=slim
VERSION=1.3.2

DEFINES=-DPACKAGE=\"$(NAME)\" -DVERSION=\"$(VERSION)\" \
		-DPKGDATADIR=\"$(PREFIX)/share/slim\" -DSYSCONFDIR=\"$(CFGDIR)\"

OBJECTS=jpeg.o png.o main.o image.o numlock.o cfg.o switchuser.o app.o panel.o util.o
ifdef USE_PAM
OBJECTS+=PAM.o
endif

all: slim

slim: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(NAME)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CUSTOM) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(DEFINES) $(CUSTOM) -c $< -o $@

install: slim install-theme
	install -D -m 755 slim $(DESTDIR)$(PREFIX)/bin/slim
	install -D -m 644 slim.1 $(DESTDIR)$(MANDIR)/man1/slim.1
	test -e $(DESTDIR)$(CFGDIR)/slim.conf || \
		install -D -m 644 slim.conf $(DESTDIR)$(CFGDIR)/slim.conf

clean:
	@rm -f slim *.o

dist:
	@rm -rf $(NAME)-$(VERSION)
	@mkdir $(NAME)-$(VERSION)
	@cp -r *.cpp *.h *.c Makefile Makefile.* COPYING ChangeLog INSTALL README TODO \
		xinitrc.sample slim.1 THEMES themes slim.conf $(NAME)-$(VERSION)
	@rm -rf $(NAME)-$(VERSION)/themes/.svn	$(NAME)-$(VERSION)/themes/default/.svn
	@tar cvzf $(NAME)-$(VERSION).tar.gz $(NAME)-$(VERSION)
	@rm -rf $(NAME)-$(VERSION)


install-theme:
	install -D -m 644 themes/default/slim.theme \
		 $(DESTDIR)$(PREFIX)/share/slim/themes/default/slim.theme
	install -D -m 644 themes/default/panel.png \
		 $(DESTDIR)$(PREFIX)/share/slim/themes/default/panel.png
	install -D -m 644 themes/default/background.jpg \
		 $(DESTDIR)$(PREFIX)/share/slim/themes/default/background.jpg
