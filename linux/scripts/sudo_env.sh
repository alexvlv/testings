#! /bin/sh

sudo -i env | sort > /tmp/env.sudo_i
sudo su -c 'env | sort' > /tmp/env.sudo_su
diff -u /tmp/env.sudo_i /tmp/env.sudo_su
