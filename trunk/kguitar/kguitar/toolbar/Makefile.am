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
