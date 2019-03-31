# umaskd

__umaskd__ is an event-driven (uses inotify) daemon which enforces per-directory permissions. Although the shell has priority over a file's permissions via `umask`, this daemon can be used to specify minimum and maximum permissions on each new file in a particular directory.

This is useful in several cases:

1. You regularly `scp` files into `~/public_html`, only to discover that web visitors receive a `403 Forbidden` error as the permissions are incorrect. __umaskd__ can monitor `~/public_html` for new files and set the permissions to be at least 0444 (i.e. readable by everyone).

2. You regularly transfer files over NFS or other mechanism to your home directory, but would like to keep them private. In this case, __umask__ can ensure that the permissions are no more than 0770 (i.e. read/write/execute by user and group, but no permissions for others).

Furthermore, you can combine these cases, creating a private home directory with a public subdirectory.


## Configuration File

By default, the `${prefix}/etc/umaskd.conf` file contains directory umask
settings. Using the -f or --file option, a different file can be specified.

The format of the configuration file is one directory setting per row. Each
row should contain a three or four digit minimum umask followed by whitespace
followed by a three or four digit maximum umask followed by whitespace and
then the directory path. For example:

    0133 0022 /home/chris/public_html

This would boolean-OR each new file's permissions with 644, and boolean-AND
with 755.
