#!/bin/bash

#  Copyright 2017 The WebRTC project authors. All Rights Reserved.
#
#  Use of this source code is governed by a BSD-style license
#  that can be found in the LICENSE file in the root of the source
#  tree. An additional intellectual property rights grant can be found
#  in the file PATENTS.  All contributing project authors may
#  be found in the AUTHORS file in the root of the source tree.

# This script has been rewritten in Python. Temporary "redirect":

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

p=`pwd`
exec "$SCRIPT_DIR/build_linux_libs.py" "$@"
