#!/bin/sh

test -n "$srcdir" || srcdir=$(dirname "$0") #$0：表示传递给shell脚本的第0个参数,dirname获取脚本所在路径
test -n "$srcdir" || srcdir=.

olddir=$(pwd)  # 保存当前工作目录
cd "$srcdir"   # 切换到脚本所在工作目录 

if ! (autoreconf --version >/dev/null 2>&1); then # 判断autoreconf是否存在 命令执行成功返回true
        echo "*** No autoreconf found, please install it ***"
        exit 1
fi

mkdir -p m4

autoreconf --force --install --verbose # 执行autoreconf

cd "$olddir" # 切换到之前的工作目录
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@" #不存在NOCONFIGURE环境变量 执行configure $@表示传入所以参数
