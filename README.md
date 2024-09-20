```markdown
# CMInjector

A simple C++ command-line application that injects a DLL into the highest priority process with a given filename on Windows, using Windows API.

## Features

- Retrieves the process ID of the highest priority instance of a specified process.
- Copies a specified DLL file from the current directory to the system's temporary directory.
- Injects the DLL into the target process using a remote thread.

## Prerequisites

- Windows operating system.
- Visual Studio or any C++ compiler that supports C++20.
- Windows SDK installed.

## Usage

1. Clone the repository:
```
   git clone https://github.com/CrunchyRL/CMInjector.git
   cd CMinjector
```
2. Build the project using your preferred IDE or compiler.

3. Run the executable with the following command:

   ```bash
   dll-injector <Process Name> <DLL Filename>
   ```

   - `<Process Name>`: The name of the target process (e.g., `RocketLeague.exe`).
   - `<DLL Filename>`: The name of the DLL file you want to inject (e.g., `Voltage.dll`).

### Example

```bash
CMInjector RocketLeague.exe Voltage.dll
```

## Important Notes

- Ensure that the target process is running before attempting to inject the DLL.
- The application may require administrative privileges to open other processes.
- Use this tool responsibly and only with processes you own or have permission to modify.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more information.

## Acknowledgments

- Windows API documentation for insights on process and memory management.
```

### Customize:
- Replace `yourusername` in the Git clone command with your actual GitHub username.
- You might want to add any additional instructions, acknowledgments, or license details as necessary.
