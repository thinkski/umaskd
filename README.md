# umaskd v1.0

__umaskd__ is an event-driven (uses inotify) utility which implements
per-directory umasks. Although the shell has priority over a file's umask,
this daemon can be used to specify minimum and maximum permission sets on
each new file in a particular directory.

For example, say you're on a large shared system and hence set your default
umask to 0077. However, you would like any files transferred via scp to your
public_html/ subdirectory to have a umask of 0022. This is where umaskd can
help. Via a configuration file, you specify a minimum permission mask as well
as a maximum permission mask for a particular directory, such as public_html/.

A minimum as well as a maximum are required since the shell first applies its
umask, and you may want a particular directory's mask to be more or less
restrictive than this default.

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
