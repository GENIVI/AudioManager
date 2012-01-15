#!/bin/bash

PACKAGE_NAME=$1
PACKAGE_NAME_PATH=$2/$PACKAGE_NAME
TMPDIR=`mktemp -d /tmp/gen.XXXXXXXXXX` || exit 1
dpkg-deb -x "$PACKAGE_NAME_PATH" "$TMPDIR"
dpkg-deb --control "$PACKAGE_NAME_PATH" "$TMPDIR"/DEBIAN
cat "$TMPDIR"/DEBIAN/control | sed -e "s/Depends.*$/Depends: $3/" > "$TMPDIR"/DEBIAN/control_
cp "$TMPDIR"/DEBIAN/control_ "$TMPDIR"/DEBIAN/control
dpkg -b "$TMPDIR" "$PACKAGE_NAME_PATH"
rm -r "$TMPDIR"

