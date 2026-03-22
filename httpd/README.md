# httpd

An HTTP/1.1 static file server written in C99, running as a POSIX daemon.

## What it does

- Serves static files over HTTP/1.1
- Parses configuration from a file (port, root directory, log settings)
- Forks into the background and runs as a proper POSIX daemon
- Handles GET requests with correct status codes, headers, and MIME types

## Architecture

```
src/
├── main.c              # Entry point
├── config/             # Configuration file parsing
├── daemon/             # Process daemonization
├── server/             # Socket setup, accept loop
├── http/               # HTTP request parsing and response
└── utils/string/       # String utilities
```

## Build

```bash
make
```

## Usage

```bash
./httpd -c config.txt
```

Example `config.txt`:
```
ServerRoot /var/www/html
Listen 8080
```

## Tests

```bash
make check
```

---

EPITA — Systems programming (ING1)
