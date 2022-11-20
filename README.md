xfce4-xkb-plugin - An Xfce4 XKB Layouts panel plugin.
===================
https://docs.xfce.org/panel-plugins/xfce4-xkb-plugin

Information
===========

This plugin allows you to setup and use multiple (currently
up to 4 due to X11 protocol limitation) keyboard layouts.

You can choose the keyboard model, what key combination to
use to switch between the layouts, the actual keyboard layouts,
the way in which the current layout is being displayed (country
flag image or text) and the layout policy, which is whether to
store the layout globally (for all windows), per application or
per window. If the policy is per application or per window, then
for each layout you can specify a comma-separated list of window
class names which will default to using that layout.

If a certain country's flag is missing, the plugin will fallback
to displaying the layout as text.

The plugin detects any change in the layout configuration
(e.g. setxkbmap invocations) and reconfigures itself to use
the new settings.

The plugin shows a small black circle with a thin white
outline in the bottom right corner of the flag image if the
current layout is the second variant configured for some
language. If the display mode is set to "text" then a little
dot is displayed as a subscript of the layout text.

Known limitations and bugs
==========================

Currently if one sets any Xkb options, besides the grp: ones
they will be lost the next time the plugin is started. Even
more - only the first grp: option present in the running
configuration will be stored in the config file and thus only
it will be restored the next time the plugin is started. This
will be resolved in future versions.

Contact
=======

Send any question, suggestions, etc. to me, Azamat H. Hackimov
(azamat.hackimov@gmail.com).


How to report bugs?
===================

You can report bugs and feature requests at 
https://gitlab.xfce.org/panel-plugins/xfce4-xkb-plugin/.
You will need to create an account for yourself.

You can also join the Xfce development mailing-list:
https://mail.xfce.org/mailman/listinfo/xfce4-dev
