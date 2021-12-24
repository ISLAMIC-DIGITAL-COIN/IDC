ISLAMIC DIGITAL COIN
=============

Setup
---------------------
[ISLAMIC DIGITAL COIN](http://islamic_digital_coin.org/wallet) is the original ISLAMIC_DIGITAL_COIN client and it builds the backbone of the network. However, it downloads and stores the entire history of ISLAMIC_DIGITAL_COIN transactions; depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more. Thankfully you only have to do this once.

Running
---------------------
The following are some helpful notes on how to run ISLAMIC DIGITAL COIN on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/islamic_digital_coin-qt` (GUI) or
- `bin/islamic_digital_coind` (headless)

If this is the first time running ISLAMIC DIGITAL COIN (since v5.0.0), you'll need to install the sapling params by running the included `install-params.sh` script, which copies the two params files to `$HOME/.islamic_digital_coin-params`

### Windows

Unpack the files into a directory, and then run islamic_digital_coin-qt.exe.

### macOS

Drag ISLAMIC_DIGITAL_COIN-Qt to your applications folder, and then run ISLAMIC_DIGITAL_COIN-Qt.

### Need Help?

* See the documentation at the [ISLAMIC_DIGITAL_COIN Wiki](https://github.com/ISLAMIC_DIGITAL_COIN-Project/ISLAMIC_DIGITAL_COIN/wiki)
for help and more information.
* Ask for help on [BitcoinTalk](https://bitcointalk.org/index.php?topic=1262920.0) or on the [ISLAMIC_DIGITAL_COIN Forum](http://forum.islamic_digital_coin.org/).
* Join our Discord server [Discord Server](https://discord.islamic_digital_coin.org)

Building
---------------------
The following are developer notes on how to build ISLAMIC DIGITAL COIN on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The ISLAMIC_DIGITAL_COIN repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Multiwallet Qt Development](multiwallet-qt.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://www.fuzzbawls.pw/islamic_digital_coin/doxygen/)
- [Translation Process](translation_process.md)
- [Unit Tests](unit-tests.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Dnsseed Policy](dnsseed-policy.md)

### Resources
* Discuss on the [BitcoinTalk](https://bitcointalk.org/index.php?topic=1262920.0) or the [ISLAMIC_DIGITAL_COIN](http://forum.islamic_digital_coin.org/) forum.
* Join the [ISLAMIC_DIGITAL_COIN Discord](https://discord.islamic_digital_coin.org).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Reduce Memory](reduce-memory.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
