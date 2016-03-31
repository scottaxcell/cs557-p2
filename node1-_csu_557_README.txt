# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

"""
Copyright (c) 2014-2015,  Regents of the University of California,
                          Arizona Board of Regents,
                          Colorado State University,
                          University Pierre & Marie Curie, Sorbonne University,
                          Washington University in St. Louis,
                          Beijing Institute of Technology

This file is part of NFD (Named Data Networking Forwarding Daemon).
See AUTHORS.md for complete list of NFD authors and contributors.

NFD is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
"""


VERSION='0.1'
APPNAME='ndn-cs557'

from waflib import Configure, Utils, Logs, Context
import os

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])

    opt.load(['default-compiler-flags', 'boost'],
              tooldir=['.waf-tools'])

    opts = opt.add_option_group('producer Options')
    opts.add_option('--debug', action='store_true', default=False, dest='debug',
                    help='''debug mode''')

def configure(conf):
    conf.load(['compiler_cxx', 'default-compiler-flags', 'boost', 'gnu_dirs'])

    conf.find_program('bash', var='BASH')

    if not os.environ.has_key('PKG_CONFIG_PATH'):
        os.environ['PKG_CONFIG_PATH'] = ':'.join([
            '/usr/local/lib/pkgconfig',
            '/opt/local/lib/pkgconfig'])

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CXX', mandatory=True)

    boost_libs = 'system random thread filesystem'

    conf.check_boost(lib=boost_libs, mandatory=True)
    if conf.env.BOOST_VERSION_NUMBER < 104800:
        Logs.error("Minimum required boost version is 1.48.0")
        Logs.error("Please upgrade your distribution or install custom boost libraries" +
                    " (http://redmine.named-data.net/projects/nfd/wiki/Boost_FAQ)")
        return

    conf.write_config_header('config.h')

def build (bld):
    bld(
        target="bin/fshare",
        features=['cxx', 'cxxprogram'],
        source=bld.path.ant_glob(['fshare/*.cpp']),
        includes=".",
        use="NDN_CXX BOOST"
    )
=86	localhostoperatorKEYksk-1459018342727ID-CERTý