EXTRA_DIST = chord.xpm note1.xpm note2.xpm note4.xpm note8.xpm note16.xpm \
 note32.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_datadir)/kguitar/toolbar/
	$(INSTALL_DATA) chord.xpm $(kde_datadir)/kguitar/toolbar/chord.xpm
	$(INSTALL_DATA) note1.xpm $(kde_datadir)/kguitar/toolbar/note1.xpm
	$(INSTALL_DATA) note2.xpm $(kde_datadir)/kguitar/toolbar/note2.xpm
	$(INSTALL_DATA) note4.xpm $(kde_datadir)/kguitar/toolbar/note4.xpm
	$(INSTALL_DATA) note8.xpm $(kde_datadir)/kguitar/toolbar/note8.xpm
	$(INSTALL_DATA) note16.xpm $(kde_datadir)/kguitar/toolbar/note16.xpm
	$(INSTALL_DATA) note32.xpm $(kde_datadir)/kguitar/toolbar/note32.xpm
	$(INSTALL_DATA) timesig.xpm $(kde_datadir)/kguitar/toolbar/timesig.xpm

uninstall-local:
	-rm -f $(kde_datadir)/kguitar/toolbar/chord.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/note1.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/note2.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/note4.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/note8.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/note16.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/note32.xpm
	-rm -f $(kde_datadir)/kguitar/toolbar/timesig.xpm
