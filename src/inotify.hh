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
#include <set>
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

        // Verbosity flag
        bool verbose_;

        // Maps watch descriptor to umask
        std::map <int, unsigned short> andmask_;
        std::map <int, unsigned short> ormask_;

        // Maps watch descriptor to path
        std::map <int, std::string> path_;

        // Set of watch descriptor to temporarily ignore. As we're watching
        // for attribute changes, but making attribute changes, cause
        // infinite loop otherwise.
        std::set <std::string> ignore_;

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

            // By default, verbosity disabled
            verbose_ = false;
        }

        ///
        /// Add path to watchlist
        ///
        /// @param path Path to watch, as a string (no trailing slash).
        /// @param ormask User file-creation minimum mask, in octal.
        /// @param andmask User file-creation maximum mask, in octal.
        ///
        /// @return 0 on success, -1 on error.
        ///
        int add_path(const char *path,
                     unsigned short ormask,
                     unsigned short andmask)
        {
            // Add path to watchlist of our inotify instance
            int wd = inotify_add_watch(fd_, path,
                IN_ONLYDIR | IN_CREATE | IN_MOVED_TO | IN_ATTRIB);
            if (wd < 0) {
                perror("inotify_add_watch");
                return -1;
            }

            // Add mask to key-value table
            andmask_.insert(std::pair<int, unsigned short>(wd, andmask));
            ormask_.insert(std::pair<int, unsigned short>(wd, ormask));

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
                struct inotify_event *event;
                for (int i = 0; i < n; i += EVENT_SIZE + event->len) {
                    // Get event pointer
                    event = (struct inotify_event *) &event_buf_[i];

                    // If in ignore set, remove and continue
                    if (ignore_.erase(event->name) > 0) {
                    	continue;
					}

                    // Change working directory to path
                    if (chdir(path_[event->wd].c_str()) < 0) {
                        perror("chdir");
                        continue;
                    }

                    // Get current permissions. On error, skip this event.
                    if (stat(event->name, st_buf_) < 0) {
                        perror("lstat");
                        continue;
                    }

                    // Mask permissions
                    mode_t mode = (
                    	st_buf_->st_mode | (~ormask_[event->wd] & 0777)
					) & (~andmask_[event->wd]);
                    ignore_.insert(event->name);
                    if (chmod(event->name, mode) < 0) {
                        perror("chmod");
                        continue;
                    }
                    // Informational message
                    if (verbose_) {
                        fprintf(stderr, "info: chmod %04o %s\n",
							mode, event->name
                        );
                    }
                }
            }

            // Free memory. Should never get here.
        }

        ///
        /// Sets verbosity
        ///
        /// @param verbose If true, information messages will be printed.
        ///
        void set_verbose(bool verbose) {
            verbose_ = verbose;
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
