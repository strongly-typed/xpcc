#!/usr/bin/env python
# 
# Copyright (c) 2013, Roboterclub Aachen e.V.
# All Rights Reserved.
#
# The file is part of the xpcc library and is released under the 3-clause BSD
# license. See the file `LICENSE` for the full license governing this code.

import sys, os, platform
from SCons.Script import *

# add python module from tools to path
# this is apparently not pythonic, but I see no other way to do this
# without polluting the site_tools directory or haveing duplicate code
sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', 'tools', 'logger'))
from logger import Logger

# -----------------------------------------------------------------------------
def logger_debug(env, s, alias='logger_debug'):
	env['XPCC_LOGGER'].debug(s)

# -----------------------------------------------------------------------------
def logger_info(env, s, alias='logger_info'):
	env['XPCC_LOGGER'].info(s)

# -----------------------------------------------------------------------------
def logger_warn(env, s, alias='logger_warn'):
	env['XPCC_LOGGER'].warn(s)

# -----------------------------------------------------------------------------
def logger_error(env, s, alias='logger_error'):
	env['XPCC_LOGGER'].error(s)

# -----------------------------------------------------------------------------
def logger_set_log_level(env, new_level, alias='logger_set_log_level'):
	env['XPCC_LOGGER'].setLogLevel(new_level)

# -----------------------------------------------------------------------------
def logger_is_log_level(env, log_level, alias='logger_is_log_level'):
	env['XPCC_LOGGER'].isLogLevel(log_level)

# -----------------------------------------------------------------------------
def logger_get_logger(env, alias='logger_is_log_level'):
	return env['XPCC_LOGGER']

# -----------------------------------------------------------------------------
def generate(env, **kw):
	env['XPCC_LOGGER'] = Logger()
	env.AddMethod(logger_debug, 'Debug')
	env.AddMethod(logger_info,  'Info')
	env.AddMethod(logger_warn,  'Warn')
	env.AddMethod(logger_error, 'Error')
	env.AddMethod(logger_set_log_level, 'SetLogLevel')
	env.AddMethod(logger_is_log_level, 'IsLogLevel')
	env.AddMethod(logger_get_logger, 'GetLogger')

def exists(env):
	return True
