# Shadowsocks VPN Client

## Overview

This project is a cross-platform VPN client for Shadowsocks. Initially, it is an experiment to create a basic client that can connect to a Shadowsocks server. The ultimate goal is to develop a fully-featured VPN client similar to Surfshark but using the Shadowsocks protocol. The client will automatically select one of our own servers and provide access via subscription.

## Features

- [x] Basic Shadowsocks client implementation
- [x] Cross-platform support (Windows, Linux)
- [ ] macOS support
- [ ] Android support
- [ ] iOS support
- [ ] Automatic server selection
- [ ] Subscription-based access
- [ ] User-friendly GUI
- [ ] Automatic updates
- [ ] Advanced logging and diagnostics
- [ ] Multi-language support

## Roadmap

### Phase 1: Basic Implementation

- [x] Set up a basic Shadowsocks client
- [x] Ensure cross-platform compatibility (Windows, Linux)
- [ ] Test and verify functionality

### Phase 2: Expand Platform Support

- [ ] Add support for macOS
- [ ] Develop Android client
- [ ] Develop iOS client

### Phase 3: Server Management

- [ ] Implement automatic server selection
- [ ] Add server management features

### Phase 4: Subscription and Authentication

- [ ] Integrate subscription-based access
- [ ] Develop authentication mechanisms

### Phase 5: User Interface

- [ ] Develop a user-friendly GUI
- [ ] Ensure consistency across platforms

### Phase 6: Additional Features

- [ ] Implement automatic updates
- [ ] Add advanced logging and diagnostics
- [ ] Multi-language support

## Getting Started

### Prerequisites

- CMake 3.10 or higher
- libsodium
- mingw-w64 (for cross-compiling to Windows)
- Wine (optional, for testing Windows binaries on Linux)

### Building the Project

#### Linux

1. Install dependencies:
    ```bash
    sudo apt update
    sudo apt install cmake libsodium-dev mingw-w64 wine
    ```

2. Clone the repository:
    ```bash
    git clone https://github.com/AlexisBlade/shadowsocks-client.git
    cd shadowsocks-client
    ```

3. Create a build directory and navigate to it:
    ```bash
    mkdir build
    cd build
    ```

4. Build the project:
    ```bash
    cmake ..
    make
    ```

5. Run the client:
    ```bash
    ./bin/shadowsocks_client
    ```

#### Windows (Cross-compiling on Linux)

1. Follow steps 1-3 from the Linux instructions.

2. Cross-compile the project:
    ```bash
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake
    make
    ```

3. The Windows executable will be located at `./bin/shadowsocks_client.exe`.

## Contribution

We welcome contributions from the community! Feel free to fork the repository and submit pull requests. Please ensure your code adheres to our coding standards and includes appropriate tests.

## License

This project is proprietary software. All rights to the source code are owned by Cherkasov Alexandr (AlexisBlade). Access to the source code is personal and having access is a significant responsibility. Any disclosure of the code is punishable by appeals to international courts or the courts of the Russian Federation.

## Contact

For questions or suggestions, please open an issue on GitHub or contact us at cherkasov.studio.official@gmail.com.
