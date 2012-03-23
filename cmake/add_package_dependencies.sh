# Copyright (C) 2012, BMW AG
#
# This file is part of GENIVI Project AudioManager.
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#

PACKAGE_NAME=$1
PACKAGE_NAME_PATH=$2/$PACKAGE_NAME
TMPDIR=`mktemp -d /tmp/gen.XXXXXXXXXX` || exit 1
dpkg-deb -x "$PACKAGE_NAME_PATH" "$TMPDIR"
dpkg-deb --control "$PACKAGE_NAME_PATH" "$TMPDIR"/DEBIAN
cat "$TMPDIR"/DEBIAN/control | sed -e "s/Depends.*$/Depends: $3/" > "$TMPDIR"/DEBIAN/control_
cp "$TMPDIR"/DEBIAN/control_ "$TMPDIR"/DEBIAN/control
dpkg -b "$TMPDIR" "$PACKAGE_NAME_PATH"
rm -r "$TMPDIR"

