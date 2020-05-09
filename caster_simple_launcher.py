#!python
# -*- coding: utf-8 -*-
from dragonfly import MappingRule, Function, Window, RunCommand

from castervoice.lib.ctrl.mgr.rule_details import RuleDetails
from castervoice.lib.merge.state.short import R

eyemouse_handle = None

def _create_hidden_window():
    # modify Path to EyeXMouse.exe Location
    action = RunCommand([r"C:\\Users\\Main\\AppData\\Local\\caster\\rules\\EyeXMouse.exe"])
    action.execute()
    pid = action.process
    return pid
 
def eyemouse_launcher(kill=False):
    global eyemouse_handle
    if kill:
        if eyemouse_handle:
            eyemouse_handle.terminate()
            eyemouse_handle = None
        else:
            RunCommand(["taskkill", "/im", "eyexmouse.exe", "/f"])
    elif not eyemouse_handle:
        eyemouse_handle = _create_hidden_window()

class MouseXRule(MappingRule):

    mapping = {
        "eye mouse":
            R(Function(eyemouse_launcher, kill=False)),
        "pop eye":
            R(Function(eyemouse_launcher, kill=True)),
    }
def get_rule():
    details = RuleDetails(name="mouse x rule")
    return MouseXRule, details
