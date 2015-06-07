themedir = $(datadir)/themes/Windowck-dark/xfwm4

all: refresh all-am

refresh:
	echo -e "Going to refresh xpm files"; \
	./generator.py;

install-data: install-data-local

install-data-local:
	echo -e "Going to copy files for $$size"; \
	for file in `find . -name "*.xpm"` themerc; do \
		$(mkdir_p) $(DESTDIR)$(themedir); \
		$(install_sh_DATA) $$file $(DESTDIR)$(themedir); \
	done; 

clean: clean-data-local clean-am

clean-data-local:
	echo -e "Going to clean xpm files"; \
	rm -f *.xpm;	

uninstall-local: uninstall-am

