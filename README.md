*This project has been created as part of the 42 curriculum by ijoubair, aljbari and zzaoui.*

# Webserv

## Description

Webserv is an HTTP/1.0 web server written in C++98. The goal of the project is to understand how web servers work by implementing one from scratch without using existing HTTP server libraries.

The server reads a configuration file inspired by NGINX, listens on one or more ports, accepts client connections, parses HTTP requests, and generates appropriate HTTP responses.

It supports serving static files, executing CGI scripts, handling file uploads, custom error pages, and multiple virtual servers.

This project focuses on networking, sockets, event-driven programming, HTTP protocol implementation, process management, and software architecture.

---

## Features

- HTTP/1.0 compliant
- Non-blocking sockets
- Poll-based event loop
- Multiple virtual servers
- Configurable host and ports
- Static file serving
- GET, POST and DELETE methods
- CGI execution
- File upload support
- Directory listing (autoindex)
- Custom error pages
- Request body handling

---

## Architecture

```
Client
   │
HTTP Request
   │
   ▼
Listening Socket
   │
   ▼
Poll Event Loop
   │
   ▼
Request Parser
   │
   ├── Static File Handler
   ├── CGI Handler
   ├── Upload Handler
   └── Error Handler
   │
   ▼
HTTP Response
```

---

## Project Structure

```
.
├── Makefile
├── README.md
├── builds
├── configfile/
├── scripts/
├── src/
├── cgi-bin/
```

---

## Instructions

### Requirements

- Linux
- C++ compiler supporting C++98
- Make

### Compilation

```bash
make
```

### Run

```bash
./webserv [CONFIG_FILE]
```

### Clean

```bash
make clean
```

### Full clean

```bash
make fclean
```

### Rebuild

```bash
make re
```

---

## Configuration

The server behavior is controlled through a configuration file.

Example:

```conf
server {
    listen 8080;
    server_name localhost;
    root ./www;
    index index.html;

    location /uploads {
        upload_store ./uploads;
    }

    error_page 404 ./errors/404.html;
}
```

---

## Supported HTTP Methods

| Method |       Description         |
|--------|---------------------------|
| GET    | Retrieve resources        |
| POST   | Send data and execute CGI |
| DELETE | Delete resources          |

---

## CGI

The server supports CGI execution
Typical CGI workflow:

1. Client sends request.
2. Server detects CGI.
3. Server creates a child process.
4. CGI script executes.
5. Output is returned as an HTTP response.

---

## Error Handling

The server handles common HTTP errors including:

- 400 Bad Request
- 403 Forbidden
- 404 Not Found
- 405 Method Not Allowed
- 413 Payload Too Large
- 500 Internal Server Error
- 502 Bad Gateway

---

## Resources

### HTTP

- RFC 1945 – Hypertext Transfer Protocol -- HTTP/1.0
- RFC 7230 – Message Syntax and Routing (used as a reference for chunked transfer encoding and message parsing)
- RFC 7231 – Semantics and Content (used as a reference for HTTP methods and status codes)

### C++

- https://en.cppreference.com/

### Networking

- Beej's Guide to Network Programming
- Linux `poll()` documentation
- Linux socket programming manual pages

### CGI

- CGI/1.1 Specification
- Python CGI documentation

---

## AI Usage

ChatGPT was used as a learning assistant during this project for:

- understanding HTTP protocol concepts;
- explaining socket programming and non-blocking I/O;
- clarifying the behavior of `poll()`;
- discussing CGI execution and process management;
- debugging implementation issues;
- improving project documentation.

AI was **not** used to generate the complete project implementation. All design decisions, architecture, coding, debugging, and testing were carried out by the project authors.

---

## License

This project was developed for educational purposes as part of the 42 curriculum.
