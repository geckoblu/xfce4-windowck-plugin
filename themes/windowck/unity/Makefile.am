themedir = $(datadir)/themes/Windowck/unity

all: generate all-am

generate:
	echo -e "Going to convert xpm to png files"; \
	./generator.py;

install-data: install-data-local

install-data-local:
	echo -e "Going to copy files for $$size"; \
	for file in `find . -name "*.png"`; do \
		$(mkdir_p) $(DESTDIR)$(themedir); \
		$(install_sh_DATA) $$file $(DESTDIR)$(themedir); \
	done; 

clean: clean-data-local clean-am

clean-data-local:
	echo -e "Going to clean png files"; \
	rm -f *.png *.xpm;	

uninstall-local: uninstall-am
