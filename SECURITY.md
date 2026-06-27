# Security Policy

## Project Status

`cashash.c` is a personal open-source project released under the MIT License.

The software is provided **as is**, without any warranty or guarantee. I do not guarantee that the project is free from security issues, bugs, memory-safety problems, or other defects.

That said, security reports are welcome. If a valid security issue is reported, I will try to fix it when possible.

## Supported Versions

| Version | Supported | Notes |
| ------- | --------- | ----- |
| 0.7.0   | :white_check_mark: | Added statistics and validation. |
| 0.6.0   | :white_check_mark: | Added open-addressing. |
| 0.5.0   | :x: | Added iteration over maps. |
| 0.4.0   | :x: | Added generic key support. |
| 0.3.0   | :x: | Added remove and clear API. |
| 0.2.0   | :x: | Added dynamic bucket count. |
| 0.1.0   | :x: | Initial version. Basic fixed-size string-key hash map implementation. |
| < 0.1.0 | :x: | Not supported. |

After version `0.4.0` because of change in key and functions the previous versions are not supported.

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
