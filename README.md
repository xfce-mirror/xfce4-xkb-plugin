[![License](https://img.shields.io/static/v1?label=License&message=GPL-2.0-or-later&color=blue)](https://gitlab.xfce.org/panel-plugins/xfce4-xkb-plugin/-/blob/master/COPYING)

# xfce4-xkb-plugin

An xfce4-panel plug-in that selects and displays the current keyboard layout. It provides mechanisms for switching between the layouts defined in xfce4-keyboard-settings, and maintains in the panel a display of the active layout, either as text or a flag icon.

----

### Homepage

[xfce4-xkb-plugin documentation](https://docs.xfce.org/panel-plugins/xfce4-xkb-plugin/start)

### Changelog

See [NEWS](https://gitlab.xfce.org/panel-plugins/xfce4-xkb-plugin/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[xfce4-xkb-plugin source code](https://gitlab.xfce.org/panel-plugins/xfce4-xkb-plugin)

### Download a Release Tarball

[xfce4-xkb-plugin archive](https://archive.xfce.org/src/panel-plugins/xfce4-xkb-plugin/)
    or
[xfce4-xkb-plugin tags](https://gitlab.xfce.org/panel-plugins/xfce4-xkb-plugin/-/tags)

### Installation

From source code repository: 

    $ cd xfce4-xkb-plugin
    $ meson setup build
    $ meson compile -C build
    # meson install -C build

From release tarball:

    $ tar xf xfce4-xkb-plugin-<version>.tar.xz
    $ cd xfce4-xkb-plugin-<version>
    $ meson setup build
    $ meson compile -C build
    # meson install -C build

### Uninstallation

    % ninja uninstall -C build

### Reporting Bugs

Visit the [reporting bugs](https://docs.xfce.org/panel-plugins/xfce4-xkb-plugin/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.
