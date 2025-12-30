# BitTorrent

A simple, educational **BitTorrent client written in C++**.

This project demonstrates how BitTorrent works internally by implementing the core parts of the protocol:

* Reading `.torrent` files
* Communicating with trackers
* Discovering peers
* Performing peer handshakes
* Downloading files piece-by-piece

This is **not a production-ready torrent client**. It is intended for learning, experimentation, and understanding distributed file sharing.

---

## Features

* Parse `.torrent` files
* Contact HTTP trackers
* Discover peers
* Perform BitTorrent handshakes
* Download single-file torrents
* Works on Linux, macOS, and Windows (via WSL)

---

## Requirements (Handled Automatically)

You **do not need to manually install** compilers or libraries.

The provided setup scripts automatically install everything required:

* C++ compiler
* CMake
* libcurl
* OpenSSL

---

## Project Structure (For Reference Only)

You do **not** need to modify these files to use the project.

```
BitTorrent/
├── src/                # C++ source code
├── include/            # Header files
├── build/              # Build output (created automatically)
├── sample.torrent      # Example torrent file
├── setup.sh            # Setup script for Linux/macOS
├── setup.ps1           # Setup script for Windows (WSL)
├── CMakeLists.txt
└── README.md
```

---

## Quick Start (Recommended)

### Linux / macOS

1. Open a terminal
2. Navigate into the project directory
3. Run:

```bash
chmod +x setup.sh
./setup.sh
```

This will:

* Install required dependencies
* Build the project

To download a file using the sample torrent:

```bash
./setup.sh --run-download --torrent sample.torrent --out out.bin
```

The downloaded file will be saved as `out.bin`.

---

### Windows (via WSL – Recommended)

This project relies on Linux libraries.
On Windows, it runs **inside WSL (Windows Subsystem for Linux)** automatically.

1. Open **PowerShell**
2. Navigate into the project directory
3. Run:

```powershell
.\setup.ps1
```

If WSL is not installed, the script will clearly tell you how to install it.

To download a file:

```powershell
.\setup.ps1 -RunDownload -Torrent .\sample.torrent -Out out.bin
```

---

## Using the Program (After Setup)

After setup, the compiled executable is located at:

```
build/bt_main
```

### View torrent information

```bash
./build/bt_main info sample.torrent
```

Displays:

* File name
* File size
* Piece length
* Number of pieces
* Tracker URL

---

### Get peers from the tracker

```bash
./build/bt_main peers sample.torrent
```

Displays a list of peers (`ip:port`) currently sharing the file.

---

### Perform a handshake with a peer

```bash
./build/bt_main handshake sample.torrent <ip:port>
```

Example:

```bash
./build/bt_main handshake sample.torrent 165.232.35.114:51443
```

This confirms successful communication with a peer.

---

### Download the full file

```bash
./build/bt_main download -o out.bin sample.torrent
```

* Downloads the full file described by the torrent
* Saves it as `out.bin`
* Pieces are downloaded sequentially

---

## Notes for Non-Technical Users

* This is a **command-line application**
* There is **no graphical interface**
* Download speed may be slower than popular torrent clients
* Only **single-file torrents** are supported
* This project is meant for **learning and experimentation**

---

## Why This Project Exists

This project helps you understand:

* How `.torrent` files are structured
* How `info_hash` is calculated
* How trackers and peers communicate
* How files are split into pieces and downloaded

It is useful for:

* Learning networking concepts
* Understanding BitTorrent internals
* Systems programming practice
* Interview preparation

---

## Troubleshooting

### No peers found

* The torrent may be inactive
* Try a different torrent file

### Permission denied when running setup.sh

Run:

```bash
chmod +x setup.sh
```

### Windows WSL issues

Follow the instructions printed by `setup.ps1`.
WSL installation is automatic on modern Windows systems.

---

## License

This project is provided **for educational purposes only**.
Use responsibly.
