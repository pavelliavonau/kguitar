EXTRA_DIST = chord.xpm note1.xpm note2.xpm note4.xpm note8.xpm note16.xpm \
 note32.xpm timesig.xpm arc.xpm fx-legato.xpm fx-harmonic.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_datadir)/kguitar/toolbar/
	@tbfiles='$(EXTRA_DIST)'; \
	for tbf in $$tbfiles; do \
	    $(INSTALL_DATA) $$tbf $(kde_datadir)/kguitar/toolbar/$$tbf; \
	    echo "Installing $$tbf..."; \
	done

uninstall-local:
	@tbfiles='$(EXTRA_DIST)'; \
	for tbf in $$tbfiles; do \
	    rm -f $(kde_datadir)/kguitar/toolbar/$$tbf; \
	    echo "Removing $$tbf..."; \
	done
#	-rm -f $(kde_datadir)/kguitar/toolbar/chord.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/note1.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/note2.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/note4.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/note8.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/note16.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/note32.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/timesig.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/arc.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/fx-legato.xpm
#	-rm -f $(kde_datadir)/kguitar/toolbar/fx-harmonic.xpm
