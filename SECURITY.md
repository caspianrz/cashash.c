# Security Policy

## Project Status

`cashash.c` is a personal open-source project released under the MIT License.

The software is provided **as is**, without any warranty or guarantee. I do not guarantee that the project is free from security issues, bugs, memory-safety problems, or other defects.

That said, security reports are welcome. If a valid security issue is reported, I will try to fix it when possible.

## Supported Versions

| Version | Supported | Notes |
| ------- | --------- | ----- |
| 0.1.0   | :white_check_mark: | Initial version. Basic fixed-size string-key hash map implementation. |
| < 0.1.0 | :x: | Not supported. |

Version `0.1.0` exists mainly to provide a simple working hash map with string keys, separate chaining, FNV-1a hashing, and no resizing.

## Reporting a Vulnerability

If you find a security issue, please report it privately instead of opening a public issue.

You can report vulnerabilities using GitHub’s private vulnerability reporting feature, if it is enabled for this repository.

When reporting, please include:

- A clear description of the issue
- Steps to reproduce it
- The affected version or commit
- Any proof-of-concept code, crash logs, or sanitizer output if available

I will review valid reports and try to fix accepted issues when possible. Since this is a personal project, response times are not guaranteed.

## Scope

Security issues may include, but are not limited to:

- Memory corruption
- Use-after-free
- Buffer overflows
- Undefined behavior
- Crashes caused by malformed input
- Incorrect handling of allocation failures

## License and Warranty

This project is licensed under the MIT License.

The software is provided **as is**, without warranty of any kind. See the `LICENSE` file for the full license text.
