# Client-Server Database Application

## Project Overview

This project is a multi-threaded client-server database application built in C, designed to efficiently handle data storage and retrieval operations over a network. The server handles multiple client connections concurrently using POSIX threads, allowing for scalable and responsive performance. Communication between the client and server is facilitated through TCP sockets, with support for both `PUT` and `GET` operations on a custom file-based database.

## Features

- **Multi-threaded Server:** Handles concurrent client connections using POSIX threads.
- **TCP Socket Communication:** Ensures reliable data transmission between client and server.
- **Custom File-Based Database:** Supports efficient data storage and retrieval with robust error handling.
- **Scalability:** Designed to handle multiple clients simultaneously with responsive performance.
- **Robust Error Handling:** Ensures data integrity and provides informative feedback on operations.

## Project Structure

- **server.c:** The main server-side application that manages client connections and handles `PUT` and `GET` requests.
- **client.c:** The client-side application that connects to the server and sends requests for data storage or retrieval.
- **msg.h:** Header file containing the message structure and constants used for communication between the client and server.
- **Makefile:** A makefile for compiling the server and client programs.

## How to Run

1. **Compile the Server and Client:**
   ```
   make
   ```
2. **Run the Server:**
   ```
   ./server [port_number]
   ```
3. **Run the Client:**
   ```
   ./client [server_hostname] [port_number]
   ```
4. **Interact with the Server:**
   - The client will prompt you to choose between `PUT`, `GET`, or `QUIT` operations.
   - Follow the prompts to store or retrieve records from the server's database.

## Example Usage

1. Start the server:
   ```
   ./server 8080
   ```
2. Start the client and connect to the server:
   ```
   ./client localhost 8080
   ```
3. Use the client interface to send `PUT` and `GET` requests to the server.

## License

This project is licensed under the MIT License. You are free to use, modify, and distribute this software, provided that the original copyright notice and permission notice are included in all copies or substantial portions of the Software.

```
MIT License

Copyright (c) 2024 Riley Mckinney

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Contact

For any questions or feedback, please feel free to reach out:

- Email: mckinneyriley13@gmail.com
- LinkedIn: [riley-mckinney](https://www.linkedin.com/in/riley-mckinney/)
- GitHub: [RileyHMckinney](https://github.com/RileyHMckinney)
