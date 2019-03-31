# umaskd

__umaskd__ is a daemon for enforcing per-directory file permissions.

Whenever a new file or directory is created within a monitored directory, the permissions of the new file or directory are clamped to the allowed range.

For instance, say a monitored directory has minimum permissions of 0444 (i.e. anyone can read) and maximum permissions of 0777 (i.e. anyone can read, write, and execute). A new file is created within this monitored directory which is not readable by everyone (e.g. 0640). Umaskd would immediately change the permissions of the new file to 0644.

As another example, say __umaskd__ is configured to monitor your home directory, with maximum permissions of 0770 (only accessible by user and group, not others). A file is created, perhaps copied over via NFS, with 0755 permissions. __umaskd__ immediately changes the new file's permissions to 0750.


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
