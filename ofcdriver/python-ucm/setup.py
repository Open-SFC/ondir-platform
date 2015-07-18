# Copyright  2012, 2013  Freescale Semiconductor
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#     Unless required by applicable law or agreed to in writing, software
#     distributed under the License is distributed on an "AS IS" BASIS,
#     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#     See the License for the specific language governing permissions and
#     limitations under the License.

from distutils.core import setup, Extension
import os

module1 = Extension('_ucm',
        define_macros = [('MAJOR_VERSION', '2'),('MINOR_VERSION', '0')],
        include_dirs = ['/usr/local/include','../ofcli/include/infra/transport', '../ofcli/include/infra/dm','../ofcli/include/infra/je','../ofcli/include/common','../include','../ofcli/include/lxos','../ofproto/src/include','../ext/urcu','../ext/futex','../ofcli/include/utils/netutil','../ofcli/include/utils/dslib','../ofcli/infra/je/include/', '../../cminfra/include/common', '../../cminfra/include/lxos','../../cminfra/include/utils/netutil','../../cminfra/include/utils/dslib','../../cminfra/include/infra/dm','../../cminfra/include/infra/transport','../../cminfra/include/infra/je','../../cminfra/infra/je/include'],
        libraries = ['ucmtrchl','ctlrutils','lxos','utils'],
        library_dirs = ['/usr/local/lib','../build/%s'% os.environ['LIB_BOARD_DIR'],'../build/ucmobj/%s'% os.environ['LIB_BOARD_DIR']],
        sources = ['ucm-override.c'])

setup (name = 'UCM',
        version = '2.0',
        description = 'This is a UCM package',
        author = 'Purandhar Sairam Mannidi',
        author_email = 'sairam.mp@freescale.com',
        url = 'http:/docs.python.org/extending/building',
        long_description = '''
        This is Python UCM Bindings.
        ''',
        ext_modules = [module1])
