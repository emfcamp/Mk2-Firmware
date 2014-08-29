#!/usr/bin/env python

print "started flasher"

import sys
import time
import logging
import os
import re
import subprocess
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import time

ignore_for_time = {}
ignore_duration = 3

def clean_ignore_list():
    needs_removing = []
    for path_to_be_ignored in ignore_for_time:
        if time.time() > ignore_for_time[path_to_be_ignored]:
            needs_removing.append(path_to_be_ignored)
    for path in needs_removing:
        print "Port {0} can be used again".format(path)
        ignore_for_time.pop(path, None)

class BadgeFlashingEvent(FileSystemEventHandler):

    def on_moved(self, event):
        pass

    def on_created(self, event):
        clean_ignore_list()
        path = event.src_path[5:]
        if re.match(r"^ttyACM\d+$", path):
            print path, ignore_for_time,time.time()
            if path not in ignore_for_time:
                print 'add ' + event.src_path
                command = "xterm -e " + os.path.dirname(os.path.abspath(__file__)) + "/flash_one.sh " + path
                os.system(command);
                print command
                #subprocess.call(command, )

    def on_deleted(self, event):
        path = event.src_path[5:]
        if re.match(r"^ttyACM\d+$", path):
            print 'delete ' + path
            ignore_for_time[path] = time.time() + ignore_duration;

    def on_modified(self, event):
        pass


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s - %(message)s',
                        datefmt='%Y-%m-%d %H:%M:%S')
    path = '/dev/'
    event_handler = BadgeFlashingEvent()
    observer = Observer()
    observer.schedule(event_handler, path, recursive=False)
    observer.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()