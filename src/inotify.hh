///
/// @file
/// @author Chris Hiszpanski <chiszp@gmail.com>
///
/// @section DESCRIPTION
/// This file contains the Linux inotify specific parts of umaskd.
///

#ifndef __NOTIFY_HH
#define __NOTIFY_HH

// Standard Template Library (STL)
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>

// POSIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

// Linux specific (kernel version 2.6.13 or newer)
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN    (1024 * (EVENT_SIZE + 16))

class Notify {
    private:
        struct inotify_event *event_buf_;
        struct stat *st_buf_;

        // File descriptor of inotify instance
        int fd_;

        // Maps watch descriptor to umask
        std::map <int, unsigned short> mask_;

        // Maps watch descriptor to path
        std::map <int, std::string> path_;

    public:
        ///
        /// Constructor
        ///
        Notify() {
            // Allocate memory for events
            event_buf_ = (struct inotify_event *) malloc(BUF_LEN);

            // Allocate memory for lstat() structure
            st_buf_ = (struct stat *) malloc(sizeof(struct stat));

            // Initialize inotify instance
            fd_ = inotify_init();
        }

        ///
        /// Add path to watchlist
        ///
        /// @param Path to watch, as a string (no trailing slash).
        /// @param User file-creation mask, in octal.
        ///
        /// @return 0 on success, -1 on error.
        ///
        int add_path(const char *path, unsigned short umask) {
            // Add path to watchlist of our inotify instance
            int wd = inotify_add_watch(fd_, path, IN_ONLYDIR | IN_CREATE);
            if (wd < 0) {
                perror("inotify_add_watch");
                return -1;
            }

            // Add mask to key-value table
            mask_.insert(std::pair<int, unsigned short>(wd, umask));

            // Add path to key-value table
            path_.insert(std::pair<int, std::string>(wd, std::string(path)));

            return 0;
        }

        ///
        /// Runloop
        ///
        void runloop() {
            while (1) {
                // Blocks until event(s) received
                int n = read(fd_, event_buf_, BUF_LEN);
                if (n < 0) {
                    perror("read");
                    break;
                }

                // Process all events
                int i = 0;
                while (i < n) {
                    // Get event pointer
                    struct inotify_event *event =
                        (struct inotify_event *) &event_buf_[i];

                    // Change working directory to path
                    if (chdir(path_[event->wd].c_str()) < 0) {
                        perror("chdir");
                    // Get current permissions. On error, skip this event.
                    } else if (lstat(event->name, st_buf_) < 0) {
                        perror("lstat");
                    // Change permissions
                    } else if (chmod(event->name,
                            st_buf_->st_mode & ~mask_[event->wd]) < 0) {
                        perror("chmod");
                    }

                    // Increment offset to next event
                    i += EVENT_SIZE + event->len;
                }
            }

            // Free memory. Should never get here.
        }

        ///
        /// Destructor
        ///
        ~Notify() {
            close(fd_);
            free(st_buf_);
            free(event_buf_);
        }
};

#endif // __NOTIFY_HH
