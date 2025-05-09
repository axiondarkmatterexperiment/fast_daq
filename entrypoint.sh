#!/bin/sh
set -e
echo "Cleaning up dangling containers..."
docker container prune -f
echo "Starting main process..."
exec "$@"
