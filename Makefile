# Makefile for slim - generic Linux
#
# Edit the following section to adjust the options
# to fit into your operating system / distribution
#######################################################
CXX=/usr/bin/g++
CC=/usr/bin/gcc
CFLAGS=-I. -I/usr/X11R6/include -I/usr/include/freetype2 -I/usr/include/freetype2/config -I/usr/include/libpng12 -I/usr/include
CXXLAGS=$(CFLAGS)
LDFLAGS=-L/usr/X11R6/lib -lXft -lX11 -lfreetype -lXrender -lfontconfig -lpng12 -lz -lm -lcrypt -lXpm -lXmu -lpng -ljpeg
CUSTOM=-DHAVE_SHADOW
PREFIX=/usr
CFGDIR=/etc
MANDIR=/usr/man
DESTDIR=
#######################################################

NAME=slim
VERSION=1.2.5

DEFINES=-DPACKAGE=\"$(NAME)\" -DVERSION=\"$(VERSION)\" \
		-DPKGDATADIR=\"$(PREFIX)/share/slim\" -DSYSCONFDIR=\"$(CFGDIR)\"

OBJECTS=jpeg.o png.o main.o image.o numlock.o cfg.o switchuser.o input.o app.o panel.o

all: slim

slim: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(NAME)

.cpp.o:
	$(CXX) $(CFLAGS) $(DEFINES) $(CUSTOM) -c $< -o $@

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
