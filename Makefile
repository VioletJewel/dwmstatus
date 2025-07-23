
IN = dwmstatus.c
OUT = dwmstatus
LIBS = -lX11
END = $(LIBS) $(IN) -o $(OUT)
LOCALINSTALLDIR ?= ~/.local/bin
INSTALLDIR ?= /usr/local/bin

PROD = -Ofast
DBG  = -ggdb -Wall

MSGLANGS=$(notdir $(wildcard msg/*.po))
MSGOBJS=$(addprefix locales/,$(MSGLANGS:.po=/LC_MESSAGES/dwmstatus.mo))

dwmstatus: dwmstatus.c
	clang $(PROD) $(END)

debug: dwmstatus.c
	clang $(DBG) $(END)

installlocal: dwmstatus
	mkdir -p $(LOCALINSTALLDIR)
	cp $(OUT) $(LOCALINSTALLDIR)

install: dwmstatus
	[ -d $(INSTALLDIR) ] || { >&2 echo $(INSTALLDIR) is a file, but it should be a dir; exit 2; }
	cp $(OUT) $(INSTALLDIR)

locales: $(MSGOBJS)

installlocaleslocal: $(MSGOBJS)
	for l in locales/*; do\
		n="$$HOME"/.local/share/locales/"$$(basename "$$l")"/LC_MESSAGES;\
		[ -d "$$n" ] || mkdir -p "$$n";\
		cp "$$l"/LC_MESSAGES/dwmstatus.mo "$$n";\
	done;

installlocales: $(MSGOBJS)
	for l in locales/*; do\
		n=/usr/share/locales/"$$(basename "$$l")"/LC_MESSAGES;\
		[ -d "$$n" ] || mkdir -p "$$n";\
		cp "$$l"/LC_MESSAGES/dwmstatus.mo "$$n";\
	done;

locales/%/LC_MESSAGES/dwmstatus.mo: msg/%.po
	mkdir -p "$(dir $@)"
	msgfmt -c -o $@ msg/$*.po

clean:
	rm -f dwmstatus

.PHONY: installlocal install locales installocaleslocal installlocales clean
