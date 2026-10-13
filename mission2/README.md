Automated tests:

- Help command: OK
- Status command: OK
- Missing argument: OK
- Unknown command: OK
- Too many arguments: OK
- Read test file: OK
- Missing file: OK
- Device interface unavailable: OK
- Help output: OK
- Status output: OK
- Device response content: OK
- Permission denied: OK
- Rootkit device unavailable: OK
Total: 13 passed, 0 failed.

Results are stored in test_results.txt

## Kernel Module Integration

The expected device interface for hidemod is /dev/rootkit.

The device was not present during testing on Kali Linux.

The companion program correctly detected the missing device
and returned exit code 1.

The read protocol and expected responses are not yet confirmed.

Integration tests with the real kernel module will be performed
in the LFS/QEMU environment.
