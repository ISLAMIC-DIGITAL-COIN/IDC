
Debian
====================
This directory contains files used to package islamic_digital_coind/islamic_digital_coin-qt
for Debian-based Linux systems. If you compile islamic_digital_coind/islamic_digital_coin-qt yourself, there are some useful files here.

## islamic_digital_coin: URI support ##


islamic_digital_coin-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install islamic_digital_coin-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your islamic_digital_coin-qt binary to `/usr/bin`
and the `../../share/pixmaps/islamic_digital_coin128.png` to `/usr/share/pixmaps`

islamic_digital_coin-qt.protocol (KDE)

