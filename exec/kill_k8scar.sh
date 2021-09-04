#!/bin/bash
kill -9 $(pgrep ncat)
kill -9 $(pgrep writer.sh)
kill -9 $(pgrep reader.sh)
