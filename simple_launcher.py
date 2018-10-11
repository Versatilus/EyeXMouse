#!python
# -*- coding: utf-8 -*-
from dragonfly import (Grammar, MappingRule, Function)

import subprocess

eyemouse_handle = None

def create_hidden_window(arguments):
    si = subprocess.STARTUPINFO()
    si.dwFlags = subprocess.STARTF_USESHOWWINDOW
    si.wShowWindow = subprocess.SW_HIDE
    return subprocess.Popen(
        arguments,
        close_fds=True,
        startupinfo=si,
        creationflags=subprocess.CREATE_NEW_PROCESS_GROUP)


def eyemouse_launcher(kill=False):
    global eyemouse_handle
    if kill:
        if eyemouse_handle:
            eyemouse_handle.terminate()
            eyemouse_handle = None
        else:
            create_hidden_window(["taskkill", "/im", "eyexmouse.exe", "/f"])
    elif not eyemouse_handle:
        eyemouse_handle = create_hidden_window(
            r"EyeXMouse.exe")


grammar = Grammar("mouse_launcher")

class MouseRule(MappingRule):

    mapping = {
        "Eye Mouse":
            Function(eyemouse_launcher, kill=False),
        "pop eye":
            Function(eyemouse_launcher, kill=True),
    }

grammar.add_rule(MouseRule())
grammar.load()


def unload():
    global grammar
    if grammar:
        grammar.unload()
    grammar = None
