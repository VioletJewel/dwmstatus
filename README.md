
# dwmstatus

An extremely lightweight and simple C statusbar for dwm that includes support
for sysfs, locales, battery support and shows the date and time every minute.

```
1 2 3 4 5 6 7 8 9 []=                    100.00% (Full)  Wed 23 Jul 2025  11:06
```

## Build and Install

```bash
# install locales locally if using locale other than en/C
$ make installlocaleslocal

## recommended installation (local only):
# copies ./dwmstatus to ~/.local/bin
$ make && make localinstall
# add 
$ echo "export PATH=\"\$PATH\":\"\$HOME\"/.local/bin" >> ~/.zprofile # or ~/.profile

# other installation examples:
$ LINSTALLDIR=~/scripts make localinstall          # copy to ~/scripts
$ make && sudo make install                        # copy to /usr/local/bin
$ make && INSTALLDIR=/usr/bin su -c 'make install' # copy to /usr/bin
```

## Locales Supported

en, eo, es, fr

If you want another locale, `${LC}`, for region, `${RG}`:
```bash
# msginit -i msg/messages.pot --no-translator -l ${LC}_${RG}UTF8 -o msg/${LC}.po
$ msginit -i msg/messages.pot --no-translator -l es_ES.UTF8 -o msg/es.po
# edit msg/${LC}.po and change
$ tail -n5 msg/es.po
msgid "Full"
msgstr "Llena"

msgid "Unknown"
msgstr "Desconocido"
$ make clean; make && make localinstall
```



