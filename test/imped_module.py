# Copyright 2015 by Intigua, Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

impedGlobs = 42

def hello():
    print "in imported module", impedGlobs
    print "hello"
    
class HelloCls:
    def helloo(self):
        print "in imported module class", impedGlobs
        print "clshello"