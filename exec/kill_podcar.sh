#!/bin/bash
kill -9 $(pgrep ncat)
kill -9 $(pgrep eg_tun.sh)
kill -9 $(pgrep catd)
kill -9 $(pgrep in_tun.sh)
