#!/bin/sh

echo "+=========================================================+"
echo "| Starting YOGI-Hub...                                    |"
echo "| Open http://127.0.0.1:1234/test.html to run the tests.  |"
echo "+ ========================================================+"
echo

../../yogi-hub/build/yogi-hub test-hub.json
