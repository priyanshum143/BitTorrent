# BitTorrent

A simple **BitTorrent client written in C++**.

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

The setup script will:

* Install required dependencies
* Build the project

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


## License

This project is provided **for educational purposes only**.
Use responsibly.
