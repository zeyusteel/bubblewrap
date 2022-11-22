AC_PREREQ([2.63]) # 要求autoconf最低版本,必须在AC_INIT之前定义
AC_INIT([bubblewrap], [0.7.0], [atomic-devel@projectatomic.io]) # 软件名称、版本、邮箱
AC_CONFIG_HEADER([config.h]) # 最后生成config.h文件，以便autoheader命令使用
AC_CONFIG_MACRO_DIR([m4])    # 传入宏路径,指定dir作为额外的Autoconf宏的定位
AC_CONFIG_AUX_DIR([build-aux]) # 辅助构建工具所在的目录,通常命名为build-aux

AC_USE_SYSTEM_EXTENSIONS # 使用一些扩展,会默认启用一些宏

AM_INIT_AUTOMAKE([1.11 -Wno-portability foreign no-define tar-ustar no-dist-gzip dist-xz]) # 定义automake版本和参数
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes]) #精简编译输出

AC_SYS_LARGEFILE

AC_PROG_CC      # 检查gcc
AM_PROG_CC_C_O  # 检查gcc -c -o 参数
PKG_PROG_PKG_CONFIG([])

AC_CHECK_HEADERS([sys/capability.h], [], [AC_MSG_ERROR([*** POSIX caps headers not found])]) # 检查头文件

AC_ARG_ENABLE(man,
              [AS_HELP_STRING([--enable-man],
                              [generate man pages [default=auto]])],,
              enable_man=maybe) # 是否生成man文档,会在configure中新增一个--enbale-man

AS_IF([test "$enable_man" != no], [
  AC_PATH_PROG([XSLTPROC], [xsltproc], [])
  AS_IF([test -z "$XSLTPROC"], [
    AS_IF([test "$enable_man" = yes], [
      AC_MSG_ERROR([xsltproc is required for --enable-man])
    ])
    enable_man=no
  ], [
    enable_man=yes
  ])
])
AM_CONDITIONAL(ENABLE_MAN, test "$enable_man" != no) # 让Makefile.am中使用ENABLE_MAN条件

AC_ARG_WITH([bash-completion-dir],
    AS_HELP_STRING([--with-bash-completion-dir[=PATH]],
        [Install the bash auto-completion script in this directory. @<:@default=yes@:>@]),
    [],
    [with_bash_completion_dir=yes]) # configure中使用--with-bash-completion-dir=PATH,默认PATH是yes

AS_IF([test "x$with_bash_completion_dir" = "xyes"],
  [
    PKG_CHECK_MODULES([BASH_COMPLETION], [bash-completion >= 2.0],
        [BASH_COMPLETION_DIR="`pkg-config --variable=completionsdir bash-completion`"],
        [BASH_COMPLETION_DIR="$datadir/bash-completion/completions"])
  ],
  [
    BASH_COMPLETION_DIR="$with_bash_completion_dir"
  ])

AC_SUBST([BASH_COMPLETION_DIR]) # Makefile.am添加这变量
AM_CONDITIONAL([ENABLE_BASH_COMPLETION],[test "x$with_bash_completion_dir" != "xno"]) # Makefile.am添加条件变量

AC_ARG_WITH([zsh-completion-dir],
    AS_HELP_STRING([--with-zsh-completion-dir[=PATH]],
        [Install the zsh auto-completion script in this directory. @<:@default=yes@:>@]),
    [],
    [with_zsh_completion_dir=yes])

AS_IF([test "x$with_zsh_completion_dir" = "xyes"],
  [ZSH_COMPLETION_DIR="$datadir/zsh/site-functions"],
  [ZSH_COMPLETION_DIR="$with_zsh_completion_dir"])


AC_SUBST([ZSH_COMPLETION_DIR])
AM_CONDITIONAL([ENABLE_ZSH_COMPLETION], [test "x$with_zsh_completion_dir" != "xno"])

# ------------------------------------------------------------------------------
have_selinux=no
AC_ARG_ENABLE(selinux, AS_HELP_STRING([--disable-selinux], [Disable optional SELINUX support]))
AS_IF([test "x$enable_selinux" != "xno"], [
        PKG_CHECK_MODULES([SELINUX], [libselinux >= 2.1.9],
                [AC_DEFINE(HAVE_SELINUX, 1, [Define if SELinux is available])
                 have_selinux=yes
                 M4_DEFINES="$M4_DEFINES -DHAVE_SELINUX"],
                [have_selinux=no])
        AS_IF([test "x$have_selinux" = xno && test "x$enable_selinux" = xyes],
          [AC_MSG_ERROR([*** SELinux support requested but libraries not found])])
        PKG_CHECK_MODULES([SELINUX_2_3], [libselinux >= 2.3],
                [AC_DEFINE(HAVE_SELINUX_2_3, 1, [Define if SELinux is version >= 2.3])],
                [:])
])
AM_CONDITIONAL(HAVE_SELINUX, [test "$have_selinux" = "yes"])

dnl Keep this in sync with ostree, except remove -Werror=declaration-after-statement
CC_CHECK_FLAGS_APPEND([WARN_CFLAGS], [CFLAGS], [\
        -pipe \
        -Wall \
        -Werror=shadow \
	-Werror=empty-body \
	-Werror=strict-prototypes \
	-Werror=missing-prototypes \
	-Werror=implicit-function-declaration \
        "-Werror=format=2 -Werror=format-security -Werror=format-nonliteral" \
	-Werror=pointer-arith -Werror=init-self \
        -Werror=missing-declarations \
        -Werror=return-type \
        -Werror=overflow \
        -Werror=int-conversion \
        -Werror=parenthesis \
        -Werror=incompatible-pointer-types \
        -Werror=misleading-indentation \
	-Werror=missing-include-dirs -Werror=aggregate-return \
        -Werror=switch-default \
        -Wswitch-enum \
])
AC_SUBST(WARN_CFLAGS)

AC_CHECK_LIB(cap, cap_from_text) # 检查libcap是否存在, 结果在ac_cv_lib_cap_cap_from_text变量

AS_IF([test "$ac_cv_lib_cap_cap_from_text" != "yes"],
  [AC_MSG_ERROR([*** libcap requested but not found])])

AC_ARG_WITH(priv-mode,
            AS_HELP_STRING([--with-priv-mode=setuid/none],
                           [How to set privilege-raising during make install]),
            [],
            [with_priv_mode="none"])

AM_CONDITIONAL(PRIV_MODE_SETUID, test "x$with_priv_mode" = "xsetuid")

AC_ARG_ENABLE(sudo,
              AS_HELP_STRING([--enable-sudo],[Use sudo to set privileged mode on binaries during install (only needed if --with-priv-mode used)]),
              [SUDO_BIN="sudo"], [SUDO_BIN=""])
AC_SUBST([SUDO_BIN])

AC_ARG_ENABLE(require-userns,
            AS_HELP_STRING([--enable-require-userns=yes/no (default no)],
                           [Require user namespaces by default when installed suid]),
            [],
            [enable_require_userns="no"])

AS_IF([ test "x$enable_require_userns" = "xyes" ], [
        AC_DEFINE(ENABLE_REQUIRE_USERNS, 1, [Define if userns should be used by default in suid mode])
     ])

AC_PROG_AWK # 检查awk是否存在
AC_REQUIRE_AUX_FILE([tap-driver.sh]) # 声明文件在上述定义的文件夹中必须存在

AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT # 生成config.status并启动它。configure.ac应该以调用此宏结束

echo "
    bubblewrap $VERSION
    ===================

    man pages (xsltproc):                         $enable_man
    SELinux:                                      $have_selinux
    setuid mode on make install:                  $with_priv_mode
    require default userns:                       $enable_require_userns
    mysteriously satisfying to pop:               yes"
echo ""
