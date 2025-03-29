#
# Copyright (c) [2019~2020] SigmaStar Technology.
#
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#

#!/bin/sh
srctree=$(dirname "$0")
pushd $srctree > /dev/null

mannul_sign_file=../mannul_sign_img.sh

function log_abs2pelative_path() {
    m_srctree=$(echo "$srctree" | sed 's/\/[^\/]*$//')
    replace=${m_srctree//\//\\\/}\/
    sed -i "s|$replace||g" $mannul_sign_file
    chmod +x $mannul_sign_file
}

function log_save() {
    echo "$@" >> $mannul_sign_file
}

function log_show() {
    echo "$@" >> $mannul_sign_file
}

function log_show_save() {
    echo "$@"
    echo "$@" >> $mannul_sign_file
}

function log_remove() {
    rm $mannul_sign_file -f
}
function mannul_sign_log_init() {
    log_remove
    log_save "#"
    log_save "# Copyright (c) [2019~2020] SigmaStar Technology."
    log_save "#"
    log_save "#"
    log_save "# This software is licensed under the terms of the GNU General Public"
    log_save "# License version 2, as published by the Free Software Foundation, and"
    log_save "# may be copied, distributed, and modified under those terms."
    log_save "#"
    log_save "# This program is distributed in the hope that it will be useful,"
    log_save "# but WITHOUT ANY WARRANTY; without even the implied warranty of"
    log_save "# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
    log_save "# GNU General Public License version 2 for more details."
    log_save "#"
    log_save ""
    log_save "#!/bin/sh"
    log_save ""
}
if [[ $1 == log_init ]]; then
    mannul_sign_log_init $1
fi

if [[ $1 == log_end ]]; then
    log_abs2pelative_path
fi

if [[ $1 == log_rm ]]; then
    log_remove
fi

popd > /dev/null